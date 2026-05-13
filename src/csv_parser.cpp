// ============================================================
//  csv_parser.cpp  --  Splits a CSV line by commas and stores
//                      each value in the correct CsvRow field.
// ============================================================
#include "csv_parser.h"
#include "config.h"

bool parseCsvRow(String &rawLine, CsvRow &out) {
  rawLine.trim();

  // Skip empty lines
  if (rawLine.isEmpty()) return false;

  float val[CSV_COLUMNS] = {0};
  int   idx = 0;

  // Split the line by commas, one column at a time
  while (idx < CSV_COLUMNS && rawLine.length()) {
    int    p     = rawLine.indexOf(',');
    String token = rawLine.substring(0, (p == -1) ? rawLine.length() : p);
    val[idx++]   = token.toFloat();
    rawLine      = (p == -1) ? "" : rawLine.substring(p + 1);
  }

  // Copy each value into the named struct fields
  out.sec        = (int)val[0];
  out.speed      = val[1];
  out.distRow    = val[2];
  out.accel      = val[3];
  out.slope      = val[4];
  out.coasting   = (val[5] >= 1.0f);   // 1 or more means coasting is active
  out.current    = val[6];
  out.energyInst = val[7];
  out.keLost     = val[8];
  out.eRecovered = val[9];
  out.battTemp   = val[10];

  return true;
}
