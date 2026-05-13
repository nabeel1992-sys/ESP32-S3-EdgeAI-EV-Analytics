#pragma once
#include "csv_parser.h"

// ============================================================
//  analytics.h  --  Calculates trip totals and derived values
//                   from each new CSV row.
// ============================================================

// Values that are calculated from the trip totals
struct DerivedMetrics {
  float battEff;           // how balanced charge vs discharge is (%)
  float avgWhPerKm;        // average energy used per km so far
  float regenEffAvg;       // how much braking energy was recovered (%)
  float coastingPercent;   // how much of the trip was coasting (%)
  float socPercent;        // battery charge left (%)
  float rangeLeftKm;       // estimated km remaining on current charge
  float recoveredEnergy;   // total energy recovered by regen braking (Wh)
  float extraRangeKm;      // extra range gained from regen energy (km)
};

// Call this once per CSV row to update trip totals and derived values
void updateTripMetrics(const CsvRow &row);

// Read the latest calculated values from here after calling updateTripMetrics()
extern DerivedMetrics derived;
