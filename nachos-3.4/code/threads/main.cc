#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"
#include "scheduler.h"
#include "alarm.h"
#include <string.h> // Needed for strcmp

// Declare the global variable from threadtest.cc
extern int ALGORITHM;

void ThreadTest();
void ProducerConsumerTest();
void Copy(const char *unixFile, const char *nachosFile);
void Print(const char *file);
void PerformanceTest(void);
void StartProcess(const char *file);
void StartMultipleProcess(int argc, char **argv);
void ConsoleTest(const char *in, const char *out);
void MailTest(int networkID);

int main(int argc, char **argv)
{
    int argCount;

    DEBUG('t', "Entering main");

    (void) Initialize(argc, argv);

    // --- NEW ADDITION ---
    // Pre-scan for the scheduling algorithm BEFORE ThreadTest() runs.
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-sch") && i + 1 < argc) {
            char* algoName = argv[i + 1];
            if (!strcmp(algoName, "RR")) ALGORITHM = 0;
            else if (!strcmp(algoName, "HRRN")) ALGORITHM = 1;
            else if (!strcmp(algoName, "Lottery")) ALGORITHM = 2;
            else if (!strcmp(algoName, "Priority")) ALGORITHM = 3;
            else if (!strcmp(algoName, "MLQ")) ALGORITHM = 4;
            else {
                printf("Warning: Unknown algorithm '%s'. Defaulting to MLQ.\n", algoName);
                ALGORITHM = 4;
            }
        }
    }
    // --------------------

    // Create the Alarm (timer) to enable preemption for RR and MLQ.
    // This timer will call Alarm::CallBack() on every TimerTicks.
    new Alarm(false);

#ifdef THREADS
    ThreadTest();   // finite test – completes quickly
#endif

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
        argCount = 1;
        if (!strcmp(*argv, "-z")) {
            printf("%s", copyright);
        }
        // --- NEW ADDITION ---
        // Consume the -sch flag so Nachos doesn't get confused
        else if (!strcmp(*argv, "-sch")) {
            ASSERT(argc > 1);
            argCount = 2; 
        }
        // --------------------
#ifdef USER_PROGRAM
        else if (!strcmp(*argv, "-x")) {
            ASSERT(argc > 1);
            StartMultipleProcess(argc, argv);
            argCount = 2;
        } else if (!strcmp(*argv, "-c")) {
            if (argc == 1) ConsoleTest(NULL, NULL);
            else { ASSERT(argc > 2); ConsoleTest(*(argv+1), *(argv+2)); argCount = 3; }
            interrupt->Halt();
        }
#endif
#ifdef FILESYS
        // Filesystem commands
#endif
#ifdef NETWORK
        // Network commands
#endif
    }

    currentThread->Finish();
    return 0;
}
