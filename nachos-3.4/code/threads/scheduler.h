// scheduler.h
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// Scheduling algorithm constants
#define SCHED_RR        0   // Round Robin
#define SCHED_HRRN      1   // Highest Response Ratio Next
#define SCHED_LOTTERY   2   // Lottery Scheduling
#define SCHED_PRIORITY  3   // Preemptive Priority Scheduling
#define SCHED_MLQ       4   // Multi-Level Queue (RR + FCFS)

// For MLQ demotion (number of RR quanta before moving to FCFS)
#define MAX_RR_QUANTA 3

class Scheduler {
  public:
    Scheduler();
    ~Scheduler();

    // Put a thread on the ready list (uses default queue based on policy)
    void ReadyToRun(Thread* thread);
    
    // Overloaded for MLQ: explicit queue selection (true = RR, false = FCFS)
    void ReadyToRun(Thread* thread, bool inRR);

    // Select the next thread to run according to current policy
    Thread* FindNextToRun();
    
    // Dispatch the CPU to nextThread
    void Run(Thread* nextThread);
    
    // Print the ready list(s)
    void Print();
    
    // Set the scheduling policy (call before any threads are forked)
    void SetPolicy(int policy);
    int  GetPolicy() { return currentPolicy; }
    
    // Increment waitingTime for all ready threads (called on each timer tick)
    void IncrementWaitingTimes();
    
  private:
    // For single‑queue policies (RR, HRRN, Lottery, Priority)
    List<Thread*> *readyList;
    
    // For MLQ: two queues (RR high priority, FCFS low priority)
    List<Thread*> *rrReadyList;
    List<Thread*> *fcfsReadyList;
    
    int currentPolicy;      // current scheduling algorithm
    
    // Helper methods for each algorithm
    Thread* FindNextRR();       // Round Robin (FIFO)
    Thread* FindNextHRRN();     // Highest Response Ratio Next
    Thread* FindNextLottery();  // Lottery Scheduling
    Thread* FindNextPriority(); // Preemptive Priority
    Thread* FindNextMLQ();      // Multi-Level Queue
};

#endif
