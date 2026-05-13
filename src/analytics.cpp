// ============================================================
//  analytics.cpp  --  All the maths for trip tracking.
//                     Called once per second with each new row.
// ============================================================
#include "analytics.h"
#include "shared_state.h"
#include "config.h"
#include <math.h>

DerivedMetrics derived;

void updateTripMetrics(const CsvRow &row) {

  // --- How many seconds passed since the last row ---
  int dt = row.sec - trip.lastSec;
  if (dt < 0) dt = 0;
  trip.lastSec = row.sec;

  // --- Add distance driven in this time step ---
  // speed is km/h, dt is seconds, so divide by 3600 to get km
  trip.totalDist += row.speed * (dt / 3600.0f);

  // --- Count how many seconds the driver was coasting ---
  if (row.coasting) trip.coastingSecs++;

  // --- Separate charging current from discharging current ---
  // Negative current means the battery is being charged (regen or charger)
  if (row.current < 0) trip.chargeSum    += fabs(row.current);
  else                  trip.dischargeSum += row.current;

  // --- Add energy values to running totals ---
  trip.energySum += row.energyInst;
  trip.keLostSum += row.keLost;
  trip.regenSum  += row.eRecovered;

  // --- Save latest battery temperature ---
  trip.batteryTemp = row.battTemp;

  // --- Battery efficiency: what fraction of all current went into charging ---
  float totalCurrent = trip.chargeSum + trip.dischargeSum;
  derived.battEff = (totalCurrent > 0)
    ? (trip.chargeSum / totalCurrent) * 100.0f
    : 0.0f;

  // --- Average energy per km (clamp crazy values) ---
  derived.avgWhPerKm = (trip.totalDist > 0.01f)
    ? trip.energySum / trip.totalDist
    : 0.0f;
  if (derived.avgWhPerKm > 2000.0f) derived.avgWhPerKm = 0.0f;

  // --- Smoothed energy per km using EMA ---
  // For the first 240 seconds, use the reference value to avoid bad early readings
  if (row.sec < EMA_WARMUP_SEC) {
    trip.emaWhPerKm = E_REF_WH_PER_KM;
  } else {
    // Only update when the car is actually moving
    if (row.speed > 1.0f) {
      trip.emaWhPerKm = EMA_ALPHA * derived.avgWhPerKm
                      + (1.0f - EMA_ALPHA) * trip.emaWhPerKm;
    }
    // Never let the EMA drop below the reference value
    if (trip.emaWhPerKm < E_REF_WH_PER_KM)
      trip.emaWhPerKm = E_REF_WH_PER_KM;
  }

  // --- Regen efficiency: how much braking energy did we actually recover ---
  derived.regenEffAvg = (trip.keLostSum > 0)
    ? (trip.regenSum / trip.keLostSum) * 100.0f
    : 0.0f;

  // --- Coasting percentage: what fraction of the trip was spent coasting ---
  derived.coastingPercent = (row.sec > 0)
    ? (trip.coastingSecs / (float)row.sec) * 100.0f
    : 0.0f;

  // --- State of Charge: how much battery is left ---
  float battWh        = BATT_CAPACITY_KWH * 1000.0f;  // convert kWh to Wh
  derived.socPercent  = ((battWh - trip.energySum) / battWh) * 100.0f;
  if (derived.socPercent < 0) derived.socPercent = 0.0f;  // cannot go below 0

  // --- Estimated range left based on current energy use rate ---
  derived.rangeLeftKm = (trip.emaWhPerKm > 0)
    ? (derived.socPercent / 100.0f) * battWh / trip.emaWhPerKm
    : 0.0f;

  // --- Extra range from recovered regen energy ---
  derived.recoveredEnergy = trip.regenSum;
  derived.extraRangeKm    = (derived.avgWhPerKm > 0)
    ? derived.recoveredEnergy / derived.avgWhPerKm
    : 0.0f;
}
