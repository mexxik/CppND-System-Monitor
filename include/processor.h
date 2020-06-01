#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long prev_total_j_ {0};
  long prev_active_j_ {0};
};

#endif