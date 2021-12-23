#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>


#include "linux_parser.h"

using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

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
  string os, kernel, version;
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

// DONE: Read and return the system memory utilization
// Based on feedback in discussion forum I focus only on total and available memory
float LinuxParser::MemoryUtilization() { 
  string key, value, line;
  float memtotal, memfree; 
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "MemTotal:") {
        linestream >> memtotal;
      }
      if (key == "MemFree:") {
        linestream >> memfree;
      }
    } 
  }     
  return (memtotal-memfree)/memtotal;
}

// Read and return the system uptime
// Learned later that reading the stream directly into other variable than string works as well, but did not modify
// see for example Uptime (int pid)
long LinuxParser::UpTime() { 
  // Read and return the system uptime
  long uptime;
  string line,uptimeString;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream uptimeStream(line);
    uptimeStream >> uptimeString;
  }
  uptime = std::stol(uptimeString);
  return uptime;
}



// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return ActiveJiffies()+IdleJiffies(); 
}

// Read and return the number of active jiffies for a PID
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
// + mentor help
// https://knowledge.udacity.com/questions/129844
// This function counts the active jiffies of a given process, therefore it accepts process PID as an input. 
// In the stat file of a given pid, you need to extract 14th-17th tokens 
// (please note that it is zero based indices,i.e. tokens will have the. indices from 13 to 16). 
// By summing them, you will get the active jiffies of the process (you need to convert them from string to long, then add them).
// comment: no conversion needed from string to long, if stream item is going directly in long variable time_item

long LinuxParser::ActiveJiffies(int pid) {
  long totaltime=0;
  long time_item;
  string line, item;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    // same approach here as for Uptime(int pid)
    for (int position=0;position<13;position++){linestream >> item;} // the first 12 items of the line are ignored
    for (int position=0;position<4;position++) {
      linestream >> time_item;
      totaltime+=time_item;
    }
  }    
  return totaltime / sysconf(_SC_CLK_TCK);
}


// DONE: Read and return the number of active jiffies for the system
// https://knowledge.udacity.com/questions/129844
// In the ( /proc/stat), you need to extract the first line, i.e. the line starts with (cpu). 
// You will get ten tokens. Summing them and you will get the ActiveJiffies.
// I think this description is not quite correct.
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
// what needs to be done is return the sum of the active jiffies item according to above article.

long LinuxParser::ActiveJiffies() {
  auto cpu_utilization=CpuUtilization();
  return std::stol(cpu_utilization[0]) + std::stol(cpu_utilization[1]) + std::stol(cpu_utilization[2]) 
  + std::stol(cpu_utilization[5]) + std::stol(cpu_utilization[6]) + std::stol(cpu_utilization[7]);
}

// TODO: Read and return the number of idle jiffies for the system
// adding here kidle and kIOwait, item 3 and 4 of the vector of strings 
// that CPUutilization returns
long LinuxParser::IdleJiffies() { 
  auto cpu_utilization=CpuUtilization();
  return std::stol(cpu_utilization[3])+std::stol(cpu_utilization[4]);
}

// Read and return CPU utilization
// define the function to extract all the ten values associated with the key cpu and return a vector of string
// reference explained here:  https://knowledge.udacity.com/questions/151964
// and here. https://knowledge.udacity.com/questions/547139   
// the function was not 100% clear for me. 
// it is now returning the 10 values of the line with key="cpu"
// cpu  7794150 5764 2297588 30607941 12390 0 280806 0 0 0

vector<string> LinuxParser::CpuUtilization() { 
  string line_item, line, value;
  vector<string> cpu_data;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream my_stream(line);
    my_stream >> line_item; // read "cpu"
    if (line_item == "cpu"){
      for (int  i = 0; i < 10; i++){
        my_stream >> value ;
        cpu_data.push_back(value);
      }
    } 
  }  
  return cpu_data; 
  }


// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key;
  int processes;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream my_stream(line);
      my_stream >> key;
      if (key == "processes"){
        my_stream >> processes;
      }
    }
  }
  return processes;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line, key;
  int processes;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream my_stream(line);
      my_stream >> key;
      if (key == "procs_running"){
        my_stream >> processes;
      }
    }
  }
  return processes;
}


// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
  }
  return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string key, value, line;
  float vmsize;
  int vmsize_mb;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> vmsize;
        vmsize_mb = static_cast<int>(vmsize / 1024); //converting to MBytes and changing to absolute number
      }
    }
  }
  return to_string(vmsize_mb); 
}


// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string line, key, uid;
  std::ifstream stream(kProcDirectory + to_string(pid)+ kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream my_stream(line);
      my_stream >> key;
      if (key == "Uid:"){
        my_stream >> uid;
      }
    }
  }
  return uid;
}


// Adding this helper function here to split strings based on defined delimiter
// reference: https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
// Easier solution would be to replace the ":" by space and iterate through the stream
std::vector<std::string> LinuxParser::Split_String(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}


// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string line, key, uid, user, uid_search;
  std::vector<std::string> tokens;
  user="undefined";
  uid=LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream my_stream(line);
      my_stream >> key;
      //https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
      //peter:x:1000:1000:peter,,,:/home/peter:/bin/bash
      tokens = LinuxParser::Split_String(key, ':');
      if (tokens[2]==uid) {
        user=tokens[0];
      }
    }
  }
  return user; 
}

// Read and return the uptime of a process
// Failed in Review, new version below
// Sources: 
// Project explanation material, in /proc/[pid]/stat file, the 22nd value is the starttime and should be chosen.
// https://knowledge.udacity.com/questions/182026
long LinuxParser::UpTime(int pid) {
  string line, item; // same concepts as for the other functions I wrote
  long starttime = 0;
  //same concept as before, going through the ifstream
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int position=0;position<21;position++){linestream >> item;} // the first 21 items of the line are ignored
    linestream >> starttime; //
  }
  return starttime / sysconf(_SC_CLK_TCK);
}