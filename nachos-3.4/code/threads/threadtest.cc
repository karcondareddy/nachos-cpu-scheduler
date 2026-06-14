// threadtest.cc - Multi-Algorithm Scheduling Simulation
#include "copyright.h"
#include "system.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Required for seeding the random number generator (Lottery)

#undef MAX_RR_QUANTA

#define MAX_PROCESSES 100

struct Process {
    char id[10];
    int burst;
    int arrival;
    int remaining;
    int rrQuantaUsed;      // used only for RR and MLQ
    bool inRR;             // used only for MLQ
    int waitingTime;       // used for HRRN
    int tickets;           // used for Lottery
    int priority;          // used for Priority
};

Process processes[MAX_PROCESSES];
int numProcesses = 0;
int currentTime = 0;

// Runtime parameters
int QUANTUM = 4;            // RR/Lottery quantum
int MAX_RR_QUANTUM = 2;     // demotion threshold (MLQ only)
int ALGORITHM = 4;          // 0=RR,1=HRRN,2=Lottery,3=Priority,4=MLQ

// Helper: ordinal suffix
const char* ordinal(int n) {
    if (n == 1) return "st";
    if (n == 2) return "nd";
    if (n == 3) return "rd";
    return "th";
}

// Comparison for arrival time sorting
int compareArrival(const void* a, const void* b) {
    return ((Process*)a)->arrival - ((Process*)b)->arrival;
}

// Read config.txt: quantum max_rr_quanta (ignores algorithm to allow -sch override)
void readConfig() {
    FILE* cfg = fopen("../config.txt", "r");
    if (cfg) {
        int dummyAlgo;
        if (fscanf(cfg, "%d %d %d", &QUANTUM, &MAX_RR_QUANTUM, &dummyAlgo) != 3) {
            printf("Warning: config.txt invalid. Using defaults.\n");
        } else {
            printf("Configuration loaded: quantum=%d, maxRRquanta=%d. (Command line algorithm=%d)\n",
                   QUANTUM, MAX_RR_QUANTUM, ALGORITHM);
        }
        fclose(cfg);
    } else {
        printf("No config.txt found. Using defaults (quantum=4, max quanta=2, algo=MLQ).\n");
    }
}

// ------------------------------------------------------------
// Algorithm-specific selection functions
// ------------------------------------------------------------

// Round Robin (FIFO queue)
Process* selectRR(Process** queue, int& head, int& tail) {
    if (head < tail)
        return queue[head++];
    return NULL;
}

// HRRN: select process with highest response ratio
Process* selectHRRN(Process** queue, int& head, int& tail) {
    if (head >= tail) return NULL;
    int bestIdx = -1;
    float bestRatio = -1.0;
    for (int i = head; i < tail; i++) {
        Process* p = queue[i];
        float ratio = (p->waitingTime + p->burst) / (float)p->burst;
        if (ratio > bestRatio) {
            bestRatio = ratio;
            bestIdx = i;
        }
    }
    if (bestIdx == -1) return NULL;
    Process* selected = queue[bestIdx];
    // Compact the queue
    for (int i = bestIdx; i < tail - 1; i++)
        queue[i] = queue[i+1];
    tail--;
    return selected;
}

// Lottery Scheduling
Process* selectLottery(Process** queue, int& head, int& tail) {
    if (head >= tail) return NULL;
    int totalTickets = 0;
    for (int i = head; i < tail; i++)
        totalTickets += queue[i]->tickets;
        
    if (totalTickets == 0) return queue[head++];
    int winner = rand() % totalTickets;
    int cumulative = 0;
    
    for (int i = head; i < tail; i++) {
        cumulative += queue[i]->tickets;
        if (cumulative > winner) {
            Process* selected = queue[i];
            // Remove selected
            for (int j = i; j < tail - 1; j++)
                queue[j] = queue[j+1];
            tail--;
            return selected;
        }
    }
    return queue[head++];
}

// Preemptive Priority (lower number = higher priority)
Process* selectPriority(Process** queue, int& head, int& tail) {
    if (head >= tail) return NULL;
    int bestIdx = head;
    for (int i = head + 1; i < tail; i++) {
        if (queue[i]->priority < queue[bestIdx]->priority)
            bestIdx = i;
    }
    Process* selected = queue[bestIdx];
    for (int i = bestIdx; i < tail - 1; i++)
        queue[i] = queue[i+1];
    tail--;
    return selected;
}

// ------------------------------------------------------------
// Main simulation function
// ------------------------------------------------------------
void simulate() {
    // Sort processes by arrival time
    qsort(processes, numProcesses, sizeof(Process), compareArrival);

    // For RR, HRRN, Lottery, Priority: single queue
    // Increased to 5000 to prevent overflow during heavy preemptive context switching
    Process** readyQueue = new Process*[5000];
    int readyHead = 0, readyTail = 0;

    // For MLQ: two queues (RR and FCFS)
    Process** rrQueue = new Process*[5000];
    Process** fcfsQueue = new Process*[5000];
    int rrHead = 0, rrTail = 0;
    int fcfsHead = 0, fcfsTail = 0;

    int nextArrival = 0;
    Process* current = NULL;

    // Print dynamic header based on the selected algorithm
    const char* algoNames[] = {"Round Robin", "HRRN", "Lottery", "Priority", "Multi-Level Queue"};
    printf("--- CPU Scheduling Simulation ---\n");
    
    if (ALGORITHM == 0 || ALGORITHM == 2) {
        // Lottery now uses Quanta, so we print it here
        printf("Algorithm: %s, Quantum = %d\n\n", algoNames[ALGORITHM], QUANTUM);
    } else if (ALGORITHM == 4) {
        printf("Algorithm: %s, Quantum = %d, Max RR Quanta = %d\n\n", 
               algoNames[ALGORITHM], QUANTUM, MAX_RR_QUANTUM);
    } else {
        printf("Algorithm: %s\n\n", algoNames[ALGORITHM]);
    }

    while (1) {
        // Add newly arrived processes
        if (ALGORITHM == 4) { // MLQ
            while (nextArrival < numProcesses && processes[nextArrival].arrival <= currentTime) {
                Process* p = &processes[nextArrival];
                p->inRR = true;
                p->rrQuantaUsed = 0;
                printf("Time %d: %s enters RR\n", p->arrival, p->id);
                rrQueue[rrTail++] = p;
                nextArrival++;
            }
        } else {
            while (nextArrival < numProcesses && processes[nextArrival].arrival <= currentTime) {
                Process* p = &processes[nextArrival];
                // Initialize HRRN waiting time
                if (ALGORITHM == 1) p->waitingTime = 0;
                
                printf("Time %d: %s enters ready queue\n", p->arrival, p->id);
                readyQueue[readyTail++] = p;
                nextArrival++;
            }
        }

        // Select next process
        if (ALGORITHM == 4) { // MLQ
            if (rrHead < rrTail)
                current = rrQueue[rrHead++];
            else if (fcfsHead < fcfsTail)
                current = fcfsQueue[fcfsHead++];
            else {
                if (nextArrival < numProcesses) {
                    currentTime = processes[nextArrival].arrival;
                    continue;
                } else break;
            }
        } else {
            if (readyHead < readyTail) {
                switch (ALGORITHM) {
                    case 0: current = selectRR(readyQueue, readyHead, readyTail); break;
                    case 1: current = selectHRRN(readyQueue, readyHead, readyTail); break;
                    case 2: current = selectLottery(readyQueue, readyHead, readyTail); break;
                    case 3: current = selectPriority(readyQueue, readyHead, readyTail); break;
                    default: current = selectRR(readyQueue, readyHead, readyTail);
                }
            } else {
                if (nextArrival < numProcesses) {
                    currentTime = processes[nextArrival].arrival;
                    continue;
                } else break;
            }
        }

        // Determine run time
        int start = currentTime;
        int run;
        if (ALGORITHM == 4 && current->inRR) {
            run = (current->remaining > QUANTUM) ? QUANTUM : current->remaining;
        } else if (ALGORITHM == 0 || ALGORITHM == 2) { 
            // BOTH Round Robin and Lottery are Preemptive (respect QUANTUM)
            run = (current->remaining > QUANTUM) ? QUANTUM : current->remaining;
        } else {
            // Non-preemptive algorithms or FCFS part of MLQ
            run = current->remaining;
            
            if (ALGORITHM == 3) {
                // Preemptive Priority: check if a higher priority process arrives
                for (int i = nextArrival; i < numProcesses; i++) {
                    if (processes[i].arrival >= currentTime + run) break;
                    if (processes[i].priority < current->priority) {
                        run = processes[i].arrival - currentTime;
                        break;
                    }
                }
            } else if (ALGORITHM == 4 && !current->inRR) {
                // Preemptive MLQ: FCFS gets preempted if a new process enters RR queue
                if (nextArrival < numProcesses) {
                    int nextArrTime = processes[nextArrival].arrival;
                    if (nextArrTime < currentTime + run) {
                        run = nextArrTime - currentTime;
                    }
                }
            }
        }

        currentTime += run;
        current->remaining -= run;
        int qnum = (ALGORITHM == 0 || (ALGORITHM == 4 && current->inRR)) ? current->rrQuantaUsed + 1 : 0;

        // Print execution
        if (ALGORITHM == 0 || (ALGORITHM == 4 && current->inRR)) {
            if (current->remaining == 0) {
                printf("Time %d-%d: %s executes RR (%d%s quantum) -> %s completes (CT = %d)\n",
                       start, currentTime, current->id, qnum, ordinal(qnum), current->id, currentTime);
            } else {
                printf("Time %d-%d: %s executes RR (%d%s quantum), remaining = %d\n",
                       start, currentTime, current->id, qnum, ordinal(qnum), current->remaining);
            }
            current->rrQuantaUsed++;
        } else if (ALGORITHM == 4 && !current->inRR) {
            if (current->remaining == 0) {
                printf("Time %d-%d: FCFS runs %s -> %s completes (CT = %d)\n",
                       start, currentTime, current->id, current->id, currentTime);
            } else {
                printf("Time %d-%d: FCFS runs %s (preempted), remaining = %d\n",
                       start, currentTime, current->id, current->remaining);
            }
        } else {
            // Lottery falls into this category for clean printing
            if (current->remaining == 0) {
                printf("Time %d-%d: %s executes -> %s completes (CT = %d)\n",
                       start, currentTime, current->id, current->id, currentTime);
            } else {
                printf("Time %d-%d: %s executes, remaining = %d\n",
                       start, currentTime, current->id, current->remaining);
            }
        }

        // Add arrivals that occurred during this execution
        if (ALGORITHM == 4) {
            while (nextArrival < numProcesses && processes[nextArrival].arrival <= currentTime) {
                Process* p = &processes[nextArrival];
                p->inRR = true;
                p->rrQuantaUsed = 0;
                printf("Time %d: %s enters RR\n", p->arrival, p->id);
                rrQueue[rrTail++] = p;
                nextArrival++;
            }
        } else {
            while (nextArrival < numProcesses && processes[nextArrival].arrival <= currentTime) {
                Process* p = &processes[nextArrival];
                if (ALGORITHM == 1) p->waitingTime = 0;
                printf("Time %d: %s enters ready queue\n", p->arrival, p->id);
                readyQueue[readyTail++] = p;
                nextArrival++;
            }
        }

        // Re-queue if not finished
        if (current->remaining > 0) {
            if (ALGORITHM == 4) {
                if (current->inRR) {
                    if (current->rrQuantaUsed >= MAX_RR_QUANTUM) {
                        current->inRR = false;
                        printf("Time %d: %s moved to FCFS\n", currentTime, current->id);
                        fcfsQueue[fcfsTail++] = current;
                    } else {
                        rrQueue[rrTail++] = current;
                    }
                } else {
                    // FCFS preempted - put back at front efficiently
                    fcfsQueue[--fcfsHead] = current;
                }
            } else {
                // For other algorithms (including our now preemptive Lottery)
                // simply put back at the end of the ready queue
                readyQueue[readyTail++] = current;
            }
        }

        // For HRRN, update waiting times of all ready processes
        if (ALGORITHM == 1) {
            int elapsed = currentTime - start;
            for (int i = readyHead; i < readyTail; i++)
                readyQueue[i]->waitingTime += elapsed;
        }
    }

    delete[] readyQueue;
    delete[] rrQueue;
    delete[] fcfsQueue;
}

void ReadProcessFile(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: cannot open %s\n", filename);
        return;
    }
    char line[100];
    while (fgets(line, sizeof(line), fp) && numProcesses < MAX_PROCESSES) {
        Process* p = &processes[numProcesses];
        
        // Attempt to read 4 items (ID, Burst, Arrival, Priority)
        int itemsRead = sscanf(line, "%s %d %d %d", p->id, &p->burst, &p->arrival, &p->priority);
        
        if (itemsRead >= 3) {
            p->remaining = p->burst;
            p->rrQuantaUsed = 0;
            p->inRR = true;
            p->waitingTime = 0;
            
            // If the text file only has 3 columns, default priority to 1
            if (itemsRead == 3) {
                p->priority = 1; 
            }
            
            // MAP PRIORITY TO TICKETS: Higher priority number = more tickets
            // Ensure every process gets at least 1 ticket
            p->tickets = (p->priority > 0) ? p->priority : 1;
            
            numProcesses++;
        }
    }
    fclose(fp);
}

void ThreadTest() {
    readConfig();
    ReadProcessFile("../processes.txt");
    if (numProcesses == 0) {
        printf("No processes found. Create ../processes.txt with format: ID burst arrival [priority]\n");
        return;
    }
    
    // Seed the random number generator using the current time
    // This ensures Lottery scheduling gives different sequences on every run
    srand(time(NULL));
    
    simulate();
}
