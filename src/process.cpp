#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// chosing here the way to not just pass through but store the processes in object instances
// changed this way for the new version. Only pid is set when instantiated.
// all other functions that are used in ncurses_display are then called with pid
// this comment helped a lot here
// https://knowledge.udacity.com/questions/219259

Process::Process(int pid) {
  //std::cout << to_string(pid);
  my_pid_ = pid;
}  
// Return this process's ID
int Process::Pid() {return my_pid_;}

// Return this process's CPU utilization
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
// the both total uptime of the system and uptime of the process is returned in seconds
// ActieJiffies OK, no more multiplication with 100 necessary.

float Process::CpuUtilization() const { 
  float utilization=0;
  long seconds_running = LinuxParser::UpTime() - LinuxParser::UpTime(my_pid_);
  long total_system_time = LinuxParser::ActiveJiffies(my_pid_);
  if (seconds_running!=0){utilization = float(total_system_time) / float(seconds_running);}
  return utilization;
}

// Return the command that generated this process
string Process::Command() {return LinuxParser::Command(my_pid_);}

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(my_pid_);}

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(my_pid_);}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() {return LinuxParser::UpTime(my_pid_);}


// https://knowledge.udacity.com/questions/534283
bool Process::operator<(Process const& a) const {return CpuUtilization() < a.CpuUtilization();}

