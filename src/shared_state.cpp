// ============================================================
//  shared_state.cpp  --  Create the global variables declared
//                        in shared_state.h.
//                        Every other file uses these via extern.
// ============================================================
#include "shared_state.h"
#include "config.h"

// Start with an empty JSON so the network task has something safe to read
String             sharedJson = "{}";
SemaphoreHandle_t  jsonMutex  = nullptr;  // created in setup()

// All trip totals start at zero
TripState    trip;

// All scores start at zero
ScoreState   score;

// All labels start empty — filled after first row is processed
StatusLabels labels;

// File handle and done flag
File  inFile;
bool  finished = false;
