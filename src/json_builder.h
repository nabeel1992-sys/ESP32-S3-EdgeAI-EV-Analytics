#pragma once
#include <Arduino.h>
#include "csv_parser.h"

// ============================================================
//  json_builder.h  --  Packs all the trip data into one
//                      JSON string ready to send over MQTT.
// ============================================================

// Pass the current row in and get back a complete JSON string.
// It reads derived metrics, scores, and labels automatically.
String buildJson(const CsvRow &row);
