#include "copyright.h"
#include "scheduler.h"
#include "system.h"
#include <stdlib.h>   // for rand()

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the ready lists.
//----------------------------------------------------------------------
Scheduler::Scheduler()
{
    readyList = new List<Thread*>;
    rrReadyList = new List<Thread*>;
    fcfsReadyList = new List<Thread*>;
    currentPolicy = SCHED_MLQ;   // default to MLQ (can be changed)
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the ready lists.
//----------------------------------------------------------------------
Scheduler::~Scheduler()
{
    delete readyList;
    delete rrReadyList;
    delete fcfsReadyList;
}

//----------------------------------------------------------------------
// Scheduler::SetPolicy
// 	Set the scheduling algorithm to be used.
//----------------------------------------------------------------------
void Scheduler::SetPolicy(int policy)
{
    currentPolicy = policy;
    DEBUG('t', "Scheduler policy set to %d\n", policy);
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun (single‑queue version)
// 	Put a thread on the ready list according to the current policy.
//----------------------------------------------------------------------
void Scheduler::ReadyToRun(Thread *thread)
{
    if (currentPolicy == SCHED_MLQ) {
        // MLQ uses the thread's own inRRQueue flag
        ReadyToRun(thread, thread->inRRQueue);
        return;
    }
    
    // For single‑queue policies, just append to the single ready list.
    DEBUG('t', "Putting thread %s on ready list.\n", thread->getName());
    thread->setStatus(READY);
    readyList->Append(thread);
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun (overloaded, for MLQ explicit queue selection)
// 	Put a thread onto the appropriate MLQ ready list.
//----------------------------------------------------------------------
void Scheduler::ReadyToRun(Thread *thread, bool inRR)
{
    DEBUG('t', "Putting thread %s on %s list.\n",
          thread->getName(), inRR ? "RR" : "FCFS");
    thread->setStatus(READY);
    thread->inRRQueue = inRR;
    if (inRR)
        rrReadyList->Append(thread);
    else
        fcfsReadyList->Append(thread);
}

//----------------------------------------------------------------------
// Helper: Round Robin (FIFO)
//----------------------------------------------------------------------
Thread* Scheduler::FindNextRR()
{
    if (readyList->IsEmpty())
        return NULL;
    return readyList->Remove();
}

//----------------------------------------------------------------------
// Helper: Highest Response Ratio Next (HRRN)
// 	Select thread with max (waitingTime + serviceTime) / serviceTime
//----------------------------------------------------------------------
Thread* Scheduler::FindNextHRRN()
{
    if (readyList->IsEmpty())
        return NULL;
    
    Thread *best = NULL;
    float bestRatio = -1.0;
    
    // Iterate through the list to find the highest ratio
    List<Thread*> tempList;
    Thread *t;
    while ((t = readyList->Remove()) != NULL) {
        // Avoid division by zero; serviceTime should be >0
        float ratio = (t->waitingTime + t->serviceTime) / (float)t->serviceTime;
        if (ratio > bestRatio) {
            bestRatio = ratio;
            best = t;
        }
        tempList.Append(t);
    }
    // Restore all threads except the selected one
    while ((t = tempList.Remove()) != NULL) {
        if (t != best)
            readyList->Append(t);
    }
    return best;
}

//----------------------------------------------------------------------
// Helper: Lottery Scheduling
// 	Randomly select a thread based on ticket distribution.
//----------------------------------------------------------------------
Thread* Scheduler::FindNextLottery()
{
    if (readyList->IsEmpty())
        return NULL;
    
    // First, compute total number of tickets
    int totalTickets = 0;
    List<Thread*> tempList;
    Thread *t;
    while ((t = readyList->Remove()) != NULL) {
        totalTickets += t->tickets;
        tempList.Append(t);
    }
    // Restore all threads
    while ((t = tempList.Remove()) != NULL)
        readyList->Append(t);
    
    if (totalTickets == 0)
        return FindNextRR();  // fallback
    
    // Pick a random ticket
    int winner = rand() % totalTickets;
    int cumulative = 0;
    Thread *selected = NULL;
    
    // Iterate again to find the winner
    while ((t = readyList->Remove()) != NULL) {
        cumulative += t->tickets;
        if (cumulative > winner && selected == NULL) {
            selected = t;
        }
        tempList.Append(t);
    }
    // Restore all threads except the selected one
    while ((t = tempList.Remove()) != NULL) {
        if (t != selected)
            readyList->Append(t);
    }
    return selected;
}

//----------------------------------------------------------------------
// Helper: Preemptive Priority Scheduling
// 	Select thread with smallest priority number (highest priority).
//----------------------------------------------------------------------
Thread* Scheduler::FindNextPriority()
{
    if (readyList->IsEmpty())
        return NULL;
    
    Thread *best = NULL;
    int bestPriority = 999999;  // high number = low priority
    
    List<Thread*> tempList;
    Thread *t;
    while ((t = readyList->Remove()) != NULL) {
        if (t->priority < bestPriority) {
            bestPriority = t->priority;
            best = t;
        }
        tempList.Append(t);
    }
    // Restore all threads except the selected one
    while ((t = tempList.Remove()) != NULL) {
        if (t != best)
            readyList->Append(t);
    }
    return best;
}

//----------------------------------------------------------------------
// Helper: Multi-Level Queue (MLQ)
// 	Always pick from RR queue first; if empty, from FCFS.
//----------------------------------------------------------------------
Thread* Scheduler::FindNextMLQ()
{
    if (!rrReadyList->IsEmpty())
        return rrReadyList->Remove();
    if (!fcfsReadyList->IsEmpty())
        return fcfsReadyList->Remove();
    return NULL;
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Dispatch to the appropriate algorithm's helper.
//----------------------------------------------------------------------
Thread* Scheduler::FindNextToRun()
{
    switch (currentPolicy) {
        case SCHED_RR:
            return FindNextRR();
        case SCHED_HRRN:
            return FindNextHRRN();
        case SCHED_LOTTERY:
            return FindNextLottery();
        case SCHED_PRIORITY:
            return FindNextPriority();
        case SCHED_MLQ:
            return FindNextMLQ();
        default:
            return FindNextRR();  // fallback
    }
}

//----------------------------------------------------------------------
// Scheduler::IncrementWaitingTimes
// 	Called on every timer tick (from interrupt.cc) to increase the
//      waitingTime of all threads in the ready queue(s).
//      Required for HRRN.
//----------------------------------------------------------------------
void Scheduler::IncrementWaitingTimes()
{
    if (currentPolicy == SCHED_MLQ) {
        // For MLQ, we have two queues.
        // Process RR queue
        List<Thread*> tempList;
        Thread *t;
        while ((t = rrReadyList->Remove()) != NULL) {
            t->waitingTime++;
            tempList.Append(t);
        }
        while ((t = tempList.Remove()) != NULL)
            rrReadyList->Append(t);
        // Process FCFS queue
        while ((t = fcfsReadyList->Remove()) != NULL) {
            t->waitingTime++;
            tempList.Append(t);
        }
        while ((t = tempList.Remove()) != NULL)
            fcfsReadyList->Append(t);
    } else {
        // For single‑queue policies
        List<Thread*> tempList;
        Thread *t;
        while ((t = readyList->Remove()) != NULL) {
            t->waitingTime++;
            tempList.Append(t);
        }
        while ((t = tempList.Remove()) != NULL)
            readyList->Append(t);
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//----------------------------------------------------------------------
void Scheduler::Run(Thread *nextThread)
{
    Thread *oldThread = currentThread;

#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {
        currentThread->SaveUserState();
        currentThread->space->SaveState();
    }
#endif

    oldThread->CheckOverflow();
    currentThread = nextThread;
    currentThread->setStatus(RUNNING);

    DEBUG('t', "Switching from \"%s\" to \"%s\"\n",
          oldThread->getName(), nextThread->getName());

    SWITCH(oldThread, nextThread);

    DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());

    if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
        threadToBeDestroyed = NULL;
    }

#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {
        currentThread->RestoreUserState();
        currentThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Helper function for printing a thread
//----------------------------------------------------------------------
static void ThreadPrint(Thread* t) { t->Print(); }

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the contents of the ready list(s).
//----------------------------------------------------------------------
void Scheduler::Print()
{
    if (currentPolicy == SCHED_MLQ) {
        printf("RR ready list (high priority):\n");
        rrReadyList->Apply(ThreadPrint);
        printf("\nFCFS ready list (low priority):\n");
        fcfsReadyList->Apply(ThreadPrint);
        printf("\n");
    } else {
        printf("Ready list:\n");
        readyList->Apply(ThreadPrint);
        printf("\n");
    }
}
