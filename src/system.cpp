#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a container/vector composed of the system's processes
// With help from Mentor page
// https://knowledge.udacity.com/questions/270856
//

vector<Process>& System::Processes() { 
  vector<int> pids = LinuxParser::Pids();
  processes_.clear(); // clear the vector/container of processes
  // iterate through the pids and instantiate the processes
  for (long unsigned int i = 0; i < pids.size(); i++) {
    if (!LinuxParser::Ram(pids[i]).empty()) {
      Process process(pids[i]);
      processes_.push_back(process);
    }
  }  
  // sort the processes
  std::sort(processes_.rbegin(), processes_.rend());
  return processes_;
}
 
// DONE: Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// DONE: Return the system's memory utilization
float System::MemoryUtilization() { 
    return LinuxParser::MemoryUtilization();
}

// DONE: Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem();}

// DONE: Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses();}

// DONE: Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses();}

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { 
    long uptime = LinuxParser::UpTime();
    return uptime; 
}