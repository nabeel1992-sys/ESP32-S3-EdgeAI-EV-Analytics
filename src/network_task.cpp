// ============================================================
//  network_task.cpp  --  Keeps WiFi connected and sends the
//                        latest JSON to the MQTT broker.
//                        Runs on Core 0 as a background task.
// ============================================================
#include "network_task.h"
#include "shared_state.h"
#include "config.h"

#include <WiFi.h>
#include <PubSubClient.h>

// These objects only belong to this module — nothing else needs them
static WiFiClient   espClient;
static PubSubClient mqttClient(espClient);

// --- Reconnect to WiFi if the connection dropped ---
static void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;  // already connected, do nothing

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait up to 5 seconds for connection
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// --- Reconnect to MQTT broker if the connection dropped ---
static void ensureMqtt() {
  if (mqttClient.connected()) return;  // already connected, do nothing

  // Use a random ID so we don't clash with other ESP32 clients
  String clientId = "ESP32-" + String(random(0xffff), HEX);
  mqttClient.connect(clientId.c_str());
}

// --- Main task loop (runs forever on Core 0) ---
void TaskNetwork(void *pvParameters) {
  mqttClient.setServer(MQTT_BROKER_IP, MQTT_PORT);
  mqttClient.setBufferSize(1024);

  while (true) {
    // Step 1: Make sure WiFi is up
    ensureWiFi();

    if (WiFi.status() == WL_CONNECTED) {

      // Step 2: Make sure MQTT is connected
      ensureMqtt();

      if (mqttClient.connected()) {
        mqttClient.loop();  // keep the MQTT connection alive

        // Step 3: Safely read the latest JSON from the shared buffer
        if (xSemaphoreTake(jsonMutex, (TickType_t)10) == pdTRUE) {
          String msg = sharedJson;
          xSemaphoreGive(jsonMutex);  // release the lock immediately

          // Step 4: Publish if the message looks valid (more than just "{}")
          if (msg.length() > 5) {
            mqttClient.publish(MQTT_TOPIC, msg.c_str());
          }
        }
      }
    }

    // Sleep before the next loop so we don't hog the CPU
    vTaskDelay(NETWORK_LOOP_DELAY_MS / portTICK_PERIOD_MS);
  }
}
