// ============================================================
//  ml_inference.cpp  --  Feeds sensor data into the Edge Impulse
//                        AI model and reads back the prediction.
//                        Runs at most once every 5 seconds.
// ============================================================
#include "ml_inference.h"
#include "shared_state.h"
#include "config.h"
#include "my_project_inferencing.h"

// Input array for the model (size comes from the Edge Impulse library)
static float         features[EI_CLASSIFIER_NN_INPUT_FRAME_SIZE];

// Tracks when the model last ran so we don't run it every second
static unsigned long lastInferenceMs = 0;

void runInferenceIfDue(const CsvRow &row) {

  // Check if 5 seconds have passed since the last run
  if (millis() - lastInferenceMs < INFERENCE_INTERVAL_MS) return;
  lastInferenceMs = millis();

  // Fill the feature array with the latest sensor values
  features[0] = row.speed;
  features[1] = row.distRow;
  features[2] = row.accel;
  features[3] = row.slope;
  features[4] = row.coasting ? 1.0f : 0.0f;  // 1 = coasting, 0 = not
  features[5] = row.current;
  features[6] = row.energyInst;
  features[7] = row.keLost;
  features[8] = row.eRecovered;
  features[9] = row.battTemp;

  // Wrap the array in a signal object that the model can read
  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE, &signal);

  // Run the model
  ei_impulse_result_t result = {0};
  if (run_classifier(&signal, &result, false) == EI_IMPULSE_OK) {
    // Read the output — regression gives a number, classification gives a confidence
#if defined(EI_CLASSIFIER_HAS_REGRESSION) && (EI_CLASSIFIER_HAS_REGRESSION == 1)
    trip.tempRisePred = result.regression_values[0];
#else
    trip.tempRisePred = result.classification[0].value;
#endif
  }
}
