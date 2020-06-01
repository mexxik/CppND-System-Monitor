#include <string>

#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() {
  long total = LinuxParser::Jiffies();
  long active = LinuxParser::ActiveJiffies();

  long total_delta = total - prev_total_j_;
  long active_delta = active - prev_active_j_;

  float util = (float)active_delta / (float) total_delta;

  prev_total_j_ = total;
  prev_active_j_ = active;

  return util;
}