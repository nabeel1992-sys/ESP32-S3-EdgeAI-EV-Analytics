/*********************************************************************
 * TRUE EDGE COMPUTING (DUAL CORE / MULTITASKING)
 *********************************************************************/
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include "my_project_inferencing.h" 

// --- CONFIG (Masked for Security) ---
const char* ssid        = "xxxx_YOUR_SSID_xxxx";
const char* password    = "xxxx_YOUR_PASSWORD_xxxx";
const char* mqtt_server = "xxxx_MQTT_BROKER_IP_xxxx"; 
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "trip/data";

// --- GLOBALS ---
WiFiClient   espClient;
PubSubClient client(espClient);
File         inFile;
bool         finished = false;

// Shared Data (Between Two Cores)
String sharedJson = "{}";  
SemaphoreHandle_t jsonMutex; 

// Calculation Variables
float totalDist = 0;
int   coasting_seconds = 0;
float charge_sum = 0, discharge_sum = 0;
float energy_sum = 0, regen_sum = 0, ke_lost_sum = 0;
int   lastSec = 0;
float battery_temp = 0;
float alpha = 0.1;      
float e_ref = 177.0;     
float emaWhPerKm = e_ref;

// Scores
int energy_score = 0, regen_score = 0, temp_score = 0, current_score = 0, coasting_score = 0; 

String energy_status;
String current_status;
String regen_status;

// Edge Impulse
static float features[EI_CLASSIFIER_NN_INPUT_FRAME_SIZE];
ei_impulse_result_t result = { 0 };
float temp_rise_pred = 0;
unsigned long lastInference = 0;

/*********************************************************************
 * TASK 1: NETWORK HANDLE (Core 0 - Background Worker)
 *********************************************************************/
void TaskNetwork(void *pvParameters) {
  while (true) {
    // 1. WiFi Connectivity Check
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
        vTaskDelay(500 / portTICK_PERIOD_MS); 
      }
    }

    // 2. MQTT Connectivity & Publishing
    if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
        String id = "ESP32-" + String(random(0xffff), HEX);
        client.connect(id.c_str());
      }

      if (client.connected()) {
        client.loop();
        
        // Fetch latest data and publish via MQTT
        if (xSemaphoreTake(jsonMutex, (TickType_t) 10) == pdTRUE) {
           String msgToSend = sharedJson;
           xSemaphoreGive(jsonMutex);
           
           if(msgToSend.length() > 5) {
             client.publish(mqtt_topic, msgToSend.c_str());
           }
        }
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/*********************************************************************
 * SETUP (Standard)
 *********************************************************************/
void setup() {
  Serial.begin(115200);

  // File System Init
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS failed");
    while (1) delay(100);
  }
  
  inFile = LittleFS.open("/WLTP_Class3b_1Hz_full.csv", FILE_READ);
  if (!inFile) {
    finished = true;
  } else {
    inFile.readStringUntil('\n'); // Skip CSV header
  }

  jsonMutex = xSemaphoreCreateMutex();

  WiFi.mode(WIFI_STA);
  client.setServer(mqtt_server, mqtt_port);

  // Run Network Task on Core 0
  xTaskCreatePinnedToCore(
    TaskNetwork,   
    "NetworkTask", 
    10000,         
    NULL,          
    1,             
    NULL,          
    0              
  );

  Serial.println("System Started. Processing on Core 1...");
}

/*********************************************************************
 * MAIN LOOP (Core 1 - EDGE COMPUTING)
 *********************************************************************/
void loop() {
  static unsigned long lastCalc = 0;
  
  if (finished) return;

  if (millis() - lastCalc < 1000) return;
  lastCalc = millis();

  // --- 1. DATA INGESTION ---
  if (!inFile.available()) {
    Serial.println("File processing complete");
    inFile.close();
    finished = true;
    return;
  }

  String row = inFile.readStringUntil('\n');
  row.trim();
  if (row.isEmpty()) return;

  // --- 2. CSV PARSING ---
  float val[11] = {0}; int idx = 0;
  while (idx < 11 && row.length()) {
    int p = row.indexOf(',');
    val[idx++] = row.substring(0, p==-1 ? row.length() : p).toFloat();
    row = (p==-1) ? "" : row.substring(p+1);
  }

  // --- 3. EDGE ANALYTICS LOGIC ---
  int   sec         = (int)val[0];
  float speed       = val[1];
  float distRow     = val[2];
  float accel       = val[3];
  float slope       = val[4];
  bool  coasting    = val[5] >= 1;
  float current     = val[6];
  float energy_inst = val[7];
  float ke_lost     = val[8];
  float e_recovered = val[9];
  battery_temp      = val[10];

  int dt = sec - lastSec;
  if (dt < 0) dt = 0;
  totalDist += speed * (dt / 3600.0);
  lastSec = sec;

  if (coasting) coasting_seconds++;
  if (current < 0) charge_sum += fabs(current);
  else discharge_sum += current;
  energy_sum += energy_inst;
  ke_lost_sum += ke_lost;
  regen_sum   += e_recovered;

  float batt_eff        = (charge_sum + discharge_sum > 0) ? (charge_sum / (charge_sum + discharge_sum)) * 100 : 0;
  float avgWhPerKm      = (totalDist > 0.01) ? energy_sum / totalDist : 0;
  if (avgWhPerKm > 2000) avgWhPerKm = 0;

  if (sec < 240) { emaWhPerKm = e_ref; } 
  else {
    if (speed > 1.0) { emaWhPerKm = alpha * avgWhPerKm + (1 - alpha) * emaWhPerKm; }
    if (emaWhPerKm < e_ref) emaWhPerKm = e_ref; 
  }

  float regen_eff_avg    = (ke_lost_sum > 0) ? (regen_sum / ke_lost_sum) * 100 : 0;
  float coasting_percent = (sec > 0) ? (coasting_seconds / (float)sec) * 100 : 0;

  // Driver Scoring
  energy_score  = (avgWhPerKm < 140) ? 100 : (avgWhPerKm < 180 ? 70 : 40);
  regen_score   = (regen_eff_avg >= 70) ? 100 : (regen_eff_avg >= 40 ? 70 : 40);
  temp_score    = (battery_temp >= 25 && battery_temp <= 40) ? 100 : 60;
  current_score = (batt_eff > 50) ? 100 : (batt_eff >= 20 ? 70 : 40);
  coasting_score = (coasting_percent < 5) ? 40 : (coasting_percent <= 12 ? 70 : 100);

  float driver_score = energy_score*0.35 + regen_score*0.25 + temp_score*0.15 + current_score*0.15 + coasting_score*0.10;
  
  String behavior = (driver_score >= 80) ? "Excellent" : (driver_score >= 60) ? "Moderate" : "Inefficient";
  String coasting_status = (coasting_percent < 5) ? "Low Coasting" : (coasting_percent <= 12) ? "Moderate Coasting" : "High Coasting";
  String eco_status      = coasting ? "Eco-Coasting" : "Idle";
  float recovered_energy = regen_sum;
  float extra_range_km   = (avgWhPerKm > 0) ? (recovered_energy / avgWhPerKm) : 0;

  String temp_status;
  if (!isnan(battery_temp)) {
    if (battery_temp <= 20.0f)       temp_status = "Cold";
    else if (battery_temp <= 35.0f)  temp_status = "Optimal";
    else                              temp_status = "Warm";
  } else { temp_status = "Unknown"; }

  energy_status  = (avgWhPerKm < 140) ? "Efficient" : (avgWhPerKm < 180) ? "Moderate" : "Inefficient";
  current_status = (batt_eff  > 50)  ? "High" : (batt_eff >= 20)  ? "Moderate" : "Low";
  regen_status = (regen_eff_avg >= 70) ? "Good Regen" : (regen_eff_avg >= 40) ? "Moderate Regen" : "Low Regen"; 

  // State of Charge & Range Estimation
  float batt_capacity_kWh = 52; 
  float soc_percent = ((batt_capacity_kWh * 1000.0 - energy_sum) / (batt_capacity_kWh * 1000.0)) * 100.0;
  if (soc_percent < 0) soc_percent = 0;
  float range_left_km = ((soc_percent / 100.0) * batt_capacity_kWh * 1000.0) / emaWhPerKm;

  // TinyML Inference (Edge Impulse)
  if (millis() - lastInference >= 5000) {
    lastInference = millis();
    features[0] = speed; features[1] = distRow; features[2] = accel;
    features[3] = slope; features[4] = coasting ? 1.0f : 0.0f;
    features[5] = current; features[6] = energy_inst; features[7] = ke_lost;
    features[8] = e_recovered; features[9] = battery_temp;

    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE, &signal);
    if (run_classifier(&signal, &result, false) == EI_IMPULSE_OK) {
      #if defined(EI_CLASSIFIER_HAS_REGRESSION) && (EI_CLASSIFIER_HAS_REGRESSION == 1)
        temp_rise_pred = result.regression_values[0];
      #else
        temp_rise_pred = result.classification[0].value;
      #endif
    }
  }

  // --- 4. DATA PACKAGING (JSON) ---
  int mins = sec / 60, secs = sec % 60;
  String timeLabel = String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);
  
  String json = "{";
  json += "\"time\":\"" + timeLabel + "\",";
  json += "\"speed\":" + String(speed, 1) + ",";
  json += "\"distance\":" + String(totalDist, 2) + ",";
  json += "\"acceleration\":" + String(accel, 2) + ",";
  json += "\"coasting_time\":" + String(coasting_seconds) + ",";
  json += "\"coasting_percent\":" + String(coasting_percent, 1) + ",";
  json += "\"coasting_status\":\"" + coasting_status + "\",";
  json += "\"eco_status\":\"" + eco_status + "\",";
  json += "\"batt_charge\":" + String(charge_sum, 2) + ",";
  json += "\"batt_discharge\":" + String(discharge_sum, 2) + ",";
  json += "\"batt_efficiency\":" + String(batt_eff, 2) + ",";
  json += "\"current_status\":\""+ current_status  + "\",";
  json += "\"energy_consumption\":" + String(energy_inst, 1) + ","; 
  json += "\"energy_total\":" + String(energy_sum / 1000.0, 3) + ",";
  json += "\"energy_avg\":" + String(avgWhPerKm, 2) + ",";
  json += "\"energy_status\":\""  + energy_status   + "\",";
  json += "\"regen_avg\":" + String(regen_eff_avg, 1) + ",";
  json += "\"recovered_energy\":" + String(recovered_energy, 1) + ",";
  json += "\"extra_range\":" + String(extra_range_km, 2) + ",";
  json += "\"regen_status\":\""  + regen_status   + "\",";
  json += "\"battery_temp\":" + String(battery_temp, 1) + ",";
  json += "\"temp_status\":\"" + temp_status + "\",";
  json += "\"temp_rise_pred\":" + String(temp_rise_pred, 1) + ",";
  json += "\"soc_percent\":" + String(soc_percent, 1) + ",";                
  json += "\"range_left_km\":" + String(range_left_km, 1) + ",";            
  json += "\"energy_score\":"+String(energy_score)+",";
  json += "\"regen_score\":"+String(regen_score)+",";
  json += "\"temp_score\":"+ String(temp_score)+",";
  json += "\"current_score\":"+String(current_score)+",";
  json += "\"coasting_score\":"+String(coasting_score)+",";
  json += "\"driver_score\":" + String(driver_score, 1) + ",";
  json += "\"driver_behavior\":\"" + behavior + "\"";
  json += "}";

  // --- 5. DATA HANDOVER TO NETWORK TASK ---
  if (xSemaphoreTake(jsonMutex, (TickType_t) 10) == pdTRUE) {
    sharedJson = json; 
    xSemaphoreGive(jsonMutex);
  }

  // --- 6. SERIAL DEBUG OUTPUT ---
  Serial.println(json);
}
