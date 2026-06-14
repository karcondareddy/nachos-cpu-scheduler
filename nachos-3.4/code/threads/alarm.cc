#include "copyright.h"
#include "alarm.h"
#include "system.h"
#include "scheduler.h"
#include "stats.h"

static void TimerHandler(void *arg) {
    Alarm *alarm = (Alarm *)arg;
    alarm->CallBack();
}

Alarm::Alarm(bool doRandom)
{
    timer = new Timer((VoidFunctionPtr)TimerHandler, this, doRandom);
    sleepQueue = new List<Thread*>;
}

Alarm::~Alarm()
{
    delete sleepQueue;
}

void Alarm::WaitUntil(int x)
{
    // Disable interrupts to avoid race conditions
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // Set the wakeup time (absolute tick count)
    currentThread->sleepWakeupTime = stats->totalTicks + x;

    // Insert the thread into the sleep queue (unsorted; we'll scan linearly)
    sleepQueue->Append(currentThread);

    // Put the current thread to sleep
    currentThread->Sleep();

    // Re-enable interrupts when we wake up
    interrupt->SetLevel(oldLevel);
}

void Alarm::CallBack()
{
    // 1. Wake up any threads whose sleep time has expired
    // We scan the sleep queue using a temporary list.
    List<Thread*> *temp = new List<Thread*>;
    Thread *t;
    while ((t = sleepQueue->Remove()) != NULL) {
        if (t->sleepWakeupTime <= stats->totalTicks) {
            scheduler->ReadyToRun(t);
        } else {
            temp->Append(t);
        }
    }
    // Restore the remaining sleeping threads
    while ((t = temp->Remove()) != NULL)
        sleepQueue->Append(t);
    delete temp;

    // 2. Preemption logic based on scheduling policy
    int policy = scheduler->GetPolicy();

    if (policy == SCHED_RR || (policy == SCHED_MLQ && currentThread->inRRQueue)) {
        currentThread->rrQuantaUsed++;
        if (currentThread->rrQuantaUsed >= MAX_RR_QUANTA) {
            currentThread->inRRQueue = false;
        }
        interrupt->YieldOnReturn();
    }
    else if (policy == SCHED_PRIORITY) {
        // For preemptive priority, yield on every timer tick
        interrupt->YieldOnReturn();
    }
    // For HRRN and Lottery, no preemption by default
}
