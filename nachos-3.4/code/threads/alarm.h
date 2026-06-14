// alarm.h
#ifndef ALARM_H
#define ALARM_H

#include "copyright.h"
#include "callback.h"
#include "timer.h"
#include "list.h"

// Forward declaration to avoid circular include
class Thread;

class Alarm : public CallBackObj {
  public:
    Alarm(bool doRandom);
    ~Alarm();

    // Suspend the current thread for 'x' ticks (simulate I/O wait)
    void WaitUntil(int x);

    // Called on every timer interrupt (every TimerTicks ticks)
    void CallBack();

  private:
    Timer *timer;
    List<Thread*> *sleepQueue;   // Queue of threads waiting for I/O (sorted by wakeup time)
};

#endif
