#include <string>
#include <iomanip>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long total_seconds) {
  int hours = total_seconds / 3600;
  int minutes = (total_seconds - hours * 3600) / 60;
  int seconds = total_seconds - hours * 3600 - minutes * 60;

  std::ostringstream hours_stream;
  std::ostringstream minutes_stream;
  std::ostringstream seconds_stream;

  hours_stream << std::setw(2) << std::setfill('0') << hours;
  minutes_stream << std::setw(2) << std::setfill('0') << minutes;
  seconds_stream << std::setw(2) << std::setfill('0') << seconds;

  return hours_stream.str() + ":" +
         minutes_stream.str() + ":" +
         seconds_stream.str();
}