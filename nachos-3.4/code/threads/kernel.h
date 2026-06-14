// kernel.h
//      Data structures for the overall Nachos kernel.
//
#ifndef KERNEL_H
#define KERNEL_H

class Scheduler;
class Interrupt;
class Statistics;
class Alarm;
class Thread;

// The following class defines the overall Nachos kernel.
class ThreadedKernel {
  public:
    ThreadedKernel(int argc, char **argv);
    ~ThreadedKernel();
    
    void Initialize();
    void SelfTest();
    void Run();
    
    Scheduler *scheduler;
    Interrupt *interrupt;
    Statistics *stats;
    Alarm *alarm;
    Thread *currentThread;
    Thread *threadToBeDestroyed;
};

// Global kernel object
extern ThreadedKernel *kernel;

#endif
