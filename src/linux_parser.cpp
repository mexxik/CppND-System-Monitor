#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::GetStatValue(string path, string key) {
  string line;
  std::ifstream stream(path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);

      string stat_key;
      linestream >> stat_key;

      if (stat_key == key) {
        return line.substr(line.find(key) + key.length(), line.length());
      }
    }
  }
  else {
    std::cout << "unable to open /proc/stat" << std::endl;
  }

  return "0";
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    long mem_total = 0;
    long mem_free = 0;
    long buffers = 0;
    long cached = 0;
    long s_reclaimable = 0;
    long sh_mem = 0;

    while (std::getline(stream, line)) {
      std::istringstream linestream(line);

      string key, value;
      linestream >> key >> value;

      if (key == "MemTotal:") {
        mem_total = std::stol(value);
      }
      else if (key == "MemFree:") {
        mem_free = std::stol(value);
      }
      else if (key == "Buffers:") {
        buffers = std::stol(value);
      }
      else if (key == "Cached:") {
        cached = std::stol(value);
      }
      else if (key == "SReclaimable:") {
        s_reclaimable = std::stol(value);
      }
      else if (key == "Shmem:") {
        sh_mem = std::stol(value);
      }
    }

    long total_used = mem_total - mem_free;
    long cached_memory = cached + s_reclaimable - sh_mem;
    long util = total_used - (buffers + cached_memory);

    return (float)util / (float)mem_total;
  }

  return 0.0;
}

long LinuxParser::UpTime() {
  float uptime, idle_time;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idle_time;
  }

  return long(uptime);
}

void LinuxParser::ParseJiffies(string j_string, long& j_user, long& j_nice,
                  long& j_system, long& j_idle, long& j_iowait,
                  long& j_irq, long& j_softirq) {

  std::istringstream j_stream(j_string);

  j_stream >> j_user >> j_nice >> j_system >> j_idle >> j_iowait >> j_irq >> j_softirq;
}

long LinuxParser::Jiffies() {
  string cpu_string = GetStatValue(kProcDirectory + kStatFilename, "cpu");

  long j_user, j_nice, j_system, j_idle, j_iowait, j_irq, j_softirq;

  ParseJiffies(cpu_string, j_user, j_nice,
               j_system, j_idle, j_iowait,
               j_irq, j_softirq);

  return j_user + j_nice + j_system + j_idle + j_iowait + j_irq + j_softirq;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string cpu_string = GetStatValue(kProcDirectory + "/" + to_string(pid) + kStatFilename, "cpu");

  long j_user, j_nice, j_system, j_idle, j_iowait, j_irq, j_softirq;

  ParseJiffies(cpu_string, j_user, j_nice,
               j_system, j_idle, j_iowait,
               j_irq, j_softirq);

  return j_user + j_nice + j_system;

}

long LinuxParser::ActiveJiffies() {
  string cpu_string = GetStatValue(kProcDirectory + kStatFilename, "cpu");

  long j_user, j_nice, j_system, j_idle, j_iowait, j_irq, j_softirq;

  ParseJiffies(cpu_string, j_user, j_nice,
               j_system, j_idle, j_iowait,
               j_irq, j_softirq);

  return j_user + j_nice + j_system;
}

long LinuxParser::IdleJiffies() {
  return Jiffies() - ActiveJiffies();
}

vector<long> LinuxParser::CpuUtilization(const int pid) {
  std::vector<long> processInfo = {};

  std::ifstream fileStream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  std::string line;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    const std::vector<std::string> data{
        std::istream_iterator<string>{lineStream},
        std::istream_iterator<string>{}};
    processInfo.push_back(std::stol(data[13]));
    processInfo.push_back(std::stol(data[14]));
    processInfo.push_back(std::stol(data[15]));
    processInfo.push_back(std::stol(data[16]));
    processInfo.push_back(std::stol(data[21]));

    return processInfo;
  }
  return {};
}

int LinuxParser::TotalProcesses() {
  return std::stoi(GetStatValue(kProcDirectory + kStatFilename, "processes"));
}

int LinuxParser::RunningProcesses() {
  return std::stoi(GetStatValue(kProcDirectory + kStatFilename, "procs_running"));
}

string LinuxParser::Command(int pid) {
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  string tag, value;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    return line;
  }
  return string();
}

string LinuxParser::Ram(int pid) {
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  std::string line;
  std::string tag, value;
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> tag >> value;
      if (tag == "VmSize:") {
        const long kb_ram = std::stol(value);
        const auto mb_ram = kb_ram / 1000;
        return std::to_string(mb_ram);
      }
    }
  }

  return string();
}

string LinuxParser::Uid(int pid) {
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line;
  string tag, value;
  if (fileStream.is_open()) {
    while(std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> tag >> value;
      if (tag == "Uid:")
        return value;
    }
  }

  return string();
}

string LinuxParser::User(int pid[[maybe_unused]]) {
  std::ifstream fileStream(LinuxParser::kPasswordPath);
  std::string line;
  std::string username, x, user_id;
  if (fileStream.is_open()){
    while(std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream lineStream(line);
      lineStream >> username >> x >> user_id;
      if (user_id == LinuxParser::Uid(pid)) {
        return username;
      }
    }
  }

  return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    std::vector<std::string> data{std::istream_iterator<string>{lineStream}, std::istream_iterator<string>{}};

    return LinuxParser::UpTime() - std::stol(data[21])/sysconf(_SC_CLK_TCK);
  }
  return 0;
}