// ============================================================
//  scoring.cpp  --  Scores the driver in 5 categories and
//                   combines them into one final score.
//                   Also picks the right label text for each.
// ============================================================
#include "scoring.h"
#include "shared_state.h"
#include "analytics.h"
#include "config.h"

// Helper: returns one of three scores based on two cutoff values.
// Values equal to or above goodLimit get the best score, and so on.
static int tier3(float val, float goodLimit, float modLimit,
                 int goodScore, int modScore, int badScore) {
  if (val >= goodLimit) return goodScore;
  if (val >= modLimit)  return modScore;
  return badScore;
}

void computeScores() {

  // --- Energy score: lower Wh/km is better ---
  score.energy = (derived.avgWhPerKm < ENERGY_EFFICIENT_LIMIT) ? 100
               : (derived.avgWhPerKm < ENERGY_MODERATE_LIMIT)  ?  70 : 40;

  // --- Regen score: higher recovery % is better ---
  score.regen = tier3(derived.regenEffAvg,
                      REGEN_GOOD_LIMIT, REGEN_MODERATE_LIMIT,
                      100, 70, 40);

  // --- Temperature score: best when battery is in the ideal range ---
  score.temp = (trip.batteryTemp >= TEMP_OPTIMAL_MIN &&
                trip.batteryTemp <= TEMP_OPTIMAL_MAX) ? 100 : 60;

  // --- Current score: higher charge efficiency is better ---
  score.current = (derived.battEff > BATT_EFF_HIGH_LIMIT)     ? 100
                : (derived.battEff >= BATT_EFF_MODERATE_LIMIT) ?  70 : 40;

  // --- Coasting score: more coasting = better (saves energy) ---
  score.coasting = (derived.coastingPercent < COASTING_LOW_LIMIT)  ?  40
                 : (derived.coastingPercent <= COASTING_HIGH_LIMIT) ?  70 : 100;

  // --- Final driver score: weighted average of all five scores ---
  score.driver = score.energy   * W_ENERGY
               + score.regen    * W_REGEN
               + score.temp     * W_TEMP
               + score.current  * W_CURRENT
               + score.coasting * W_COASTING;

  // --- Text label for energy use ---
  labels.energy = (derived.avgWhPerKm < ENERGY_EFFICIENT_LIMIT) ? "Efficient"
                : (derived.avgWhPerKm < ENERGY_MODERATE_LIMIT)  ? "Moderate" : "Inefficient";

  // --- Text label for battery current balance ---
  labels.current = (derived.battEff > BATT_EFF_HIGH_LIMIT)     ? "High"
                 : (derived.battEff >= BATT_EFF_MODERATE_LIMIT) ? "Moderate" : "Low";

  // --- Text label for regen braking ---
  labels.regen = (derived.regenEffAvg >= REGEN_GOOD_LIMIT)     ? "Good Regen"
               : (derived.regenEffAvg >= REGEN_MODERATE_LIMIT) ? "Moderate Regen" : "Low Regen";

  // --- Text label for coasting amount ---
  labels.coasting = (derived.coastingPercent < COASTING_LOW_LIMIT)  ? "Low Coasting"
                  : (derived.coastingPercent <= COASTING_HIGH_LIMIT) ? "Moderate Coasting"
                  : "High Coasting";

  // --- Text label for battery temperature ---
  if (!isnan(trip.batteryTemp)) {
    labels.temp = (trip.batteryTemp <= 20.0f) ? "Cold"
                : (trip.batteryTemp <= 35.0f) ? "Optimal" : "Warm";
  } else {
    labels.temp = "Unknown";  // sensor gave bad reading
  }

  // --- Overall driving behaviour label ---
  labels.behavior = (score.driver >= 80) ? "Excellent"
                  : (score.driver >= 60) ? "Moderate" : "Inefficient";
}
