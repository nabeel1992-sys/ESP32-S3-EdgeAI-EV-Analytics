#pragma once

// ============================================================
//  scoring.h  --  Gives the driver a score in each category
//                 and picks the matching text labels.
// ============================================================

// Call this after updateTripMetrics() to refresh all scores and labels
void computeScores();
