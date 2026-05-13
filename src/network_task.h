#pragma once

// ============================================================
//  network_task.h  --  Handles WiFi and MQTT on Core 0.
//                      Runs in the background so Core 1 can
//                      focus on processing without interruption.
// ============================================================

// This is the RTOS task function. Pass it to xTaskCreatePinnedToCore().
// It keeps WiFi alive and publishes the latest JSON every 100ms.
void TaskNetwork(void *pvParameters);
