// ============================================================
//  json_builder.cpp  --  Takes all current values and builds
//                        a single JSON string with every field.
// ============================================================
#include "json_builder.h"
#include "shared_state.h"
#include "analytics.h"

// Helper: add a text (string) field  -->  "key":"value"
static String kv(const String &key, const String &val, bool quoted) {
  return "\"" + key + "\":" + (quoted ? ("\"" + val + "\"") : val);
}

// Helper: add a decimal number field  -->  "key":1.23
static String kvf(const String &key, float val, int dp = 2) {
  return "\"" + key + "\":" + String(val, dp);
}

// Helper: add a whole number field  -->  "key":42
static String kvi(const String &key, int val) {
  return "\"" + key + "\":" + String(val);
}

String buildJson(const CsvRow &row) {

  // Build a time label in mm:ss format for the dashboard
  int    mins      = row.sec / 60;
  int    secs      = row.sec % 60;
  String timeLabel = String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);

  // Build the JSON string field by field
  String j = "{";
  j += kv ("time",              timeLabel,                        true) + ",";
  j += kvf("speed",             row.speed,                1)            + ",";
  j += kvf("distance",          trip.totalDist,           2)            + ",";
  j += kvf("acceleration",      row.accel,                2)            + ",";
  j += kvi("coasting_time",     trip.coastingSecs)                      + ",";
  j += kvf("coasting_percent",  derived.coastingPercent,  1)            + ",";
  j += kv ("coasting_status",   labels.coasting,                  true) + ",";
  j += kv ("eco_status",        labels.eco,                       true) + ",";
  j += kvf("batt_charge",       trip.chargeSum,           2)            + ",";
  j += kvf("batt_discharge",    trip.dischargeSum,        2)            + ",";
  j += kvf("batt_efficiency",   derived.battEff,          2)            + ",";
  j += kv ("current_status",    labels.current,                   true) + ",";
  j += kvf("energy_consumption",row.energyInst,           1)            + ",";
  j += kvf("energy_total",      trip.energySum / 1000.0f, 3)            + ","; // Wh to kWh
  j += kvf("energy_avg",        derived.avgWhPerKm,       2)            + ",";
  j += kv ("energy_status",     labels.energy,                    true) + ",";
  j += kvf("regen_avg",         derived.regenEffAvg,      1)            + ",";
  j += kvf("recovered_energy",  derived.recoveredEnergy,  1)            + ",";
  j += kvf("extra_range",       derived.extraRangeKm,     2)            + ",";
  j += kv ("regen_status",      labels.regen,                     true) + ",";
  j += kvf("battery_temp",      trip.batteryTemp,         1)            + ",";
  j += kv ("temp_status",       labels.temp,                      true) + ",";
  j += kvf("temp_rise_pred",    trip.tempRisePred,        1)            + ","; // AI prediction
  j += kvf("soc_percent",       derived.socPercent,       1)            + ",";
  j += kvf("range_left_km",     derived.rangeLeftKm,      1)            + ",";
  j += kvi("energy_score",      score.energy)                           + ",";
  j += kvi("regen_score",       score.regen)                            + ",";
  j += kvi("temp_score",        score.temp)                             + ",";
  j += kvi("current_score",     score.current)                          + ",";
  j += kvi("coasting_score",    score.coasting)                         + ",";
  j += kvf("driver_score",      score.driver,             1)            + ",";
  j += kv ("driver_behavior",   labels.behavior,                  true);
  j += "}";

  return j;
}
