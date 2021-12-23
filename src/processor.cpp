#include "processor.h"
#include "linux_parser.h"
#include <unistd.h>  // for usleep

/* Return the aggregate CPU utilization
    user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
cpu 74608   2520   24433   1117073   6176   4054  0        0      0      0

    according to: https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
    PrevIdle = previdle + previowait
    Idle = idle + iowait

    PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal
    NonIdle = user + nice + system + irq + softirq + steal

    PrevTotal = PrevIdle + PrevNonIdle
    Total = Idle + NonIdle

    # differentiate: actual value minus the previous one
    totald = Total - PrevTotal
    idled = Idle - PrevIdle

    CPU_Percentage = (totald - idled)/totald 
    Δ active time units / Δ total time units
    */

float Processor::Utilization() {
    float util;
    auto prev_active = LinuxParser::ActiveJiffies();
    auto prev_total = LinuxParser::Jiffies();
    usleep (80000);  // found this in a thread in internet
    auto active = LinuxParser::ActiveJiffies();
    auto total = LinuxParser::Jiffies();
    auto delta_total = total - prev_total;
    auto delta_active = active - prev_active; 

    if (delta_total==0) {util=0;} //avoid divsion by zero
    else {util = float(delta_active) / float(delta_total);}
    return util;
}
    