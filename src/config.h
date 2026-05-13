#pragma once

// ============================================================
//  config.h  --  All settings live here.
//                Change one value here and it updates everywhere.
// ============================================================

// --- WiFi and MQTT (network settings) ---
#define WIFI_SSID        "FRITZ!Box 7583"
#define WIFI_PASSWORD    "43649895061452487734"
#define MQTT_BROKER_IP   "192.168.178.50"
#define MQTT_PORT        1884
#define MQTT_TOPIC       "trip/data"

// --- CSV file stored on the device ---
#define CSV_PATH         "/WLTP_Class3b_1Hz_full.csv"
#define CSV_COLUMNS      11       // total columns in the CSV file

// --- Battery ---
#define BATT_CAPACITY_KWH  52.0f  // full battery size in kWh

// --- EMA smoothing (makes energy readings less jumpy) ---
#define EMA_ALPHA          0.1f   // 0 = very slow to react, 1 = instant
#define E_REF_WH_PER_KM    177.0f // default energy use before we have real data
#define EMA_WARMUP_SEC     240    // skip EMA for the first 240 seconds of the trip

// --- Energy score limits (Wh per km) ---
#define ENERGY_EFFICIENT_LIMIT   140.0f   // under 140 = efficient
#define ENERGY_MODERATE_LIMIT    180.0f   // under 180 = moderate, else bad

// --- Regen score limits (%) ---
#define REGEN_GOOD_LIMIT         70.0f    // 70% or more = good
#define REGEN_MODERATE_LIMIT     40.0f    // 40% or more = moderate

// --- Best battery temperature range (Celsius) ---
#define TEMP_OPTIMAL_MIN         25.0f
#define TEMP_OPTIMAL_MAX         40.0f

// --- Battery charge efficiency limits (%) ---
#define BATT_EFF_HIGH_LIMIT      50.0f    // over 50% = high
#define BATT_EFF_MODERATE_LIMIT  20.0f    // over 20% = moderate

// --- Coasting limits (% of total trip time) ---
#define COASTING_LOW_LIMIT        5.0f    // under 5% = low coasting
#define COASTING_HIGH_LIMIT      12.0f    // over 12% = high coasting

// --- Weight of each part in the final driver score ---
#define W_ENERGY    0.35f
#define W_REGEN     0.25f
#define W_TEMP      0.15f
#define W_CURRENT   0.15f
#define W_COASTING  0.10f

// --- AI model runs every 5 seconds ---
#define INFERENCE_INTERVAL_MS  5000

// --- Network task settings (runs on Core 0 in the background) ---
#define NETWORK_TASK_STACK     10000  // RAM given to the task
#define NETWORK_TASK_PRIORITY      1  // low priority, runs in background
#define NETWORK_TASK_CORE          0  // always use Core 0
#define NETWORK_LOOP_DELAY_MS    100  // wait 100ms between each loop
