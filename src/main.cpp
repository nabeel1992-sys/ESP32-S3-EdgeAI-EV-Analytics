// ============================================================
//  main.cpp  --  Starts everything up and runs the main loop.
//                This file just connects the modules together.
//                No maths or network logic lives here.
//
//  HOW THE MODULES FIT TOGETHER:
//
//  config.h        → all numbers and settings in one place
//  shared_state    → data shared between Core 0 and Core 1
//  network_task    → WiFi + MQTT publisher running on Core 0
//  csv_parser      → reads one CSV line into a clean struct
//  analytics       → calculates trip totals and derived values
//  scoring         → scores the driver and picks label text
//  ml_inference    → runs the AI model every 5 seconds
//  json_builder    → packs everything into one JSON string
//
//  FLOW (Core 1, once per second):
//  Read CSV row → Parse → Update analytics → Score →
//  Run AI (if due) → Build JSON → Hand off to network task
// ============================================================

#include <Arduino.h>
#include <LittleFS.h>

#include "config.h"
#include "shared_state.h"
#include "network_task.h"
#include "csv_parser.h"
#include "analytics.h"
#include "scoring.h"
#include "ml_inference.h"
#include "json_builder.h"

// ============================================================
//  SETUP  --  Runs once at boot
// ============================================================
void setup() {
  Serial.begin(115200);

  // 1. Mount the file system (CHANGED 'true' to 'false' to prevent auto-formatting)
  if (!LittleFS.begin(false)) {
    Serial.println("[ERROR] Could not mount LittleFS. Stopping.");
    while (1) delay(100);
  }

  // --- DEBUG: ESP32 MEMORY CHECK ---
  Serial.println("--- ESP32 Memory Check ---");
  File root = LittleFS.open("/");
  File memFile = root.openNextFile();
  if(!memFile) {
    Serial.println("[DEBUG] Memory is EMPTY! File did not upload.");
  }
  while(memFile){
      Serial.print("[DEBUG] Found file: ");
      Serial.println(memFile.name());
      memFile = root.openNextFile();
  }
  Serial.println("--------------------------");
  // ---------------------------------

  // 2. Open the CSV file. If it's missing, mark as finished so loop exits cleanly.
  inFile = LittleFS.open(CSV_PATH, FILE_READ);
  if (!inFile) {
    Serial.println("[WARN] CSV file not found. Nothing to process.");
    finished = true;
  } else {
    inFile.readStringUntil('\n');  // throw away the header row
  }

  // 3. Set the starting EMA value before any real data arrives
  trip.emaWhPerKm = E_REF_WH_PER_KM;

  // 4. Create the mutex that protects the shared JSON buffer
  jsonMutex = xSemaphoreCreateMutex();

  // 5. Start the network task on Core 0. It runs in the background forever.
  xTaskCreatePinnedToCore(
    TaskNetwork,
    "NetworkTask",
    NETWORK_TASK_STACK,
    NULL,
    NETWORK_TASK_PRIORITY,
    NULL,
    NETWORK_TASK_CORE
  );

  Serial.println("[INFO] Boot complete. Processing CSV on Core 1.");
}
// ============================================================
//  LOOP  --  Runs on Core 1, processes one CSV row per second
// ============================================================
void loop() {
  static unsigned long lastCalcMs = 0;

  // Nothing left to do — file is finished
  if (finished) return;

  // Only process one row per second (matches the 1 Hz CSV data rate)
  if (millis() - lastCalcMs < 1000) return;
  lastCalcMs = millis();

  // 1. Check if we have reached the end of the file
  if (!inFile.available()) {
    Serial.println("[INFO] All rows processed. Done.");
    inFile.close();
    finished = true;
    return;
  }

  // 2. Read the next line from the CSV
  String rawLine = inFile.readStringUntil('\n');

  // 3. Parse the line into a typed struct
  CsvRow row;
  if (!parseCsvRow(rawLine, row)) return;  // skip empty or broken lines

  // 4. Set the eco label (needs the raw coasting flag from the row)
  labels.eco = row.coasting ? "Eco-Coasting" : "Idle";

  // 5. Update all running totals and derived metrics
  updateTripMetrics(row);

  // 6. Calculate scores and pick text labels
  computeScores();

  // 7. Run the AI model if 5 seconds have passed since last inference
  runInferenceIfDue(row);

  // 8. Pack everything into a JSON string
  String json = buildJson(row);

  // 9. Hand the JSON to the network task safely using the mutex
  if (xSemaphoreTake(jsonMutex, (TickType_t)10) == pdTRUE) {
    sharedJson = json;
    xSemaphoreGive(jsonMutex);  // release the lock so Core 0 can read it
  }

  // 10. Print to serial for debugging
  Serial.println(json);
}
