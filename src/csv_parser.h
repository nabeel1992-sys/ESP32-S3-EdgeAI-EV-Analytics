#pragma once
#include <Arduino.h>

// ============================================================
//  csv_parser.h  --  Reads one line from the CSV file and
//                    puts each column into a named variable.
// ============================================================

// One row from the CSV file, with each column given a clear name
struct CsvRow {
  int   sec;          // time in seconds
  float speed;        // vehicle speed (km/h)
  float distRow;      // distance in this time step (km)
  float accel;        // acceleration (m/s2)
  float slope;        // road slope (degrees or %)
  bool  coasting;     // true if the driver is coasting (no throttle)
  float current;      // battery current (negative = charging)
  float energyInst;   // energy used in this time step (Wh)
  float keLost;       // kinetic energy lost during braking (Wh)
  float eRecovered;   // energy recovered by regen braking (Wh)
  float battTemp;     // battery temperature (Celsius)
};

// Give this function a raw CSV line and it fills a CsvRow for you.
// Returns false if the line is empty or cannot be parsed.
bool parseCsvRow(String &rawLine, CsvRow &out);
