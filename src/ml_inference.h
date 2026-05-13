#pragma once
#include "csv_parser.h"

// ============================================================
//  ml_inference.h  --  Runs the AI model to predict how much
//                      the battery temperature will rise.
// ============================================================

// Call this every loop. It only runs the model every 5 seconds.
// When it runs, it updates trip.tempRisePred with the prediction.
void runInferenceIfDue(const CsvRow &row);
