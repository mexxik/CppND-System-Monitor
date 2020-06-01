#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) :pid_(pid) {
  cpu_utilization_ = CpuUtilization();
}

// TODO: Return this process's ID
int Process::Pid() {
  return pid_;
}

float Process::CpuUtilization() {
  std::vector<long> info = LinuxParser::CpuUtilization(pid_);

  long total_time = info[0] + info[1];
  total_time += info[2] + info[3];

  float seconds = (float)UpTime();
  float cpu_utilization = ((total_time / sysconf(_SC_CLK_TCK)) / seconds);

  return cpu_utilization;
}


string Process::Command() {
  return LinuxParser::Command(pid_);
}

string Process::Ram() {
  return LinuxParser::Ram(pid_);
}

string Process::User() {
  return LinuxParser::User(pid_);
}

long int Process::UpTime() {
  return LinuxParser::UpTime(pid_);
}

bool Process::operator>(Process const& a) const {
  return (cpu_utilization_ > a.cpu_utilization_);
}