#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/semphr.h>

// ============================================================
//  shared_state.h  --  Data shared between Core 0 and Core 1.
//                      Both tasks read and write these values.
// ============================================================

// The latest JSON string — Core 1 writes it, Core 0 sends it
extern String             sharedJson;
extern SemaphoreHandle_t  jsonMutex;   // lock so both cores don't clash

// --- Running totals for the whole trip ---
struct TripState {
  float totalDist    = 0;   // total distance driven so far (km)
  int   coastingSecs = 0;   // total seconds spent coasting
  float chargeSum    = 0;   // total charge current collected
  float dischargeSum = 0;   // total discharge current used
  float energySum    = 0;   // total energy used so far (Wh)
  float regenSum     = 0;   // total energy recovered by braking (Wh)
  float keLostSum    = 0;   // total kinetic energy lost while braking
  float emaWhPerKm   = 0;   // smoothed energy per km (set from E_REF at start)
  int   lastSec      = 0;   // time of the last processed row (seconds)
  float batteryTemp  = 0;   // latest battery temperature reading (C)
  float tempRisePred = 0;   // AI model's predicted temperature rise
};

extern TripState trip;

// --- Score for each driving category ---
struct ScoreState {
  int   energy   = 0;   // score for energy efficiency
  int   regen    = 0;   // score for brake energy recovery
  int   temp     = 0;   // score for battery temperature
  int   current  = 0;   // score for charge/discharge balance
  int   coasting = 0;   // score for coasting behaviour
  float driver   = 0;   // final weighted driver score
};

extern ScoreState score;

// --- Text labels shown in the JSON output ---
struct StatusLabels {
  String energy;    // e.g. "Efficient", "Moderate", "Inefficient"
  String current;   // e.g. "High", "Moderate", "Low"
  String regen;     // e.g. "Good Regen", "Moderate Regen", "Low Regen"
  String temp;      // e.g. "Cold", "Optimal", "Warm"
  String coasting;  // e.g. "Low Coasting", "Moderate Coasting", "High Coasting"
  String eco;       // e.g. "Eco-Coasting" or "Idle"
  String behavior;  // e.g. "Excellent", "Moderate", "Inefficient"
};

extern StatusLabels labels;

// --- CSV file handle ---
extern File  inFile;
extern bool  finished;  // true when the whole file has been read
