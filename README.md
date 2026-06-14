# Nachos Multi-Algorithm CPU Scheduler

## Overview

This project extends the Nachos 3.4 Operating System by implementing multiple CPU scheduling algorithms and providing a Python Tkinter-based visualization dashboard for performance analysis.

The system enables users to simulate, compare, and analyze various scheduling techniques using performance metrics and graphical visualizations.

## Features

### Scheduling Algorithms Implemented

* Round Robin (RR)
* Preemptive Priority Scheduling
* High Response Ratio Next (HRRN)
* Lottery Scheduling
* Multi-Level Queue (MLQ)

### Performance Metrics

* Completion Time (CT)
* Turnaround Time (TAT)
* Waiting Time (WT)
* Average Turnaround Time
* Average Waiting Time

### Visualization Dashboard

* Interactive Python Tkinter GUI
* Dynamic Process Input
* Scheduling Configuration Panel
* Real-Time Execution Output
* Gantt Chart Visualization
* Performance Metrics Table

---

# Project Structure

```text
nachos-cpu-scheduler/
│
├── nachos-3.4/
│   ├── threads/
│   ├── machine/
│   ├── userprog/
│   └── ...
│
├── nachos-gui/
│   ├── main.py
│   ├── assets/
│   └── ...
│
├── screenshots/
│   ├── dashboard.png
│   ├── gantt-chart.png
│   └── metrics.png
│
├── docs/
│   └── project-report.pdf
│
└── README.md
```

---

# Technologies Used

## Backend

* C++
* Nachos 3.4 Operating System

## Frontend

* Python
* Tkinter

## Concepts

* CPU Scheduling
* Process Management
* Operating Systems
* Thread Scheduling
* Performance Evaluation

---

# Algorithms Implemented

## 1. Round Robin (RR)

Round Robin allocates CPU time using a fixed time quantum.

### Advantages

* Fair CPU allocation
* Prevents starvation
* Suitable for time-sharing systems

### Disadvantages

* High context-switch overhead for small quantum values
* Increased turnaround time for long processes

---

## 2. Preemptive Priority Scheduling

Processes with higher priority immediately preempt lower-priority processes.

### Advantages

* Fast response for critical tasks
* Suitable for real-time environments

### Disadvantages

* Can cause starvation of low-priority processes

---

## 3. High Response Ratio Next (HRRN)

HRRN is a non-preemptive scheduling algorithm that selects the process with the highest response ratio.

### Formula

```text
Response Ratio = (Waiting Time + Burst Time) / Burst Time
```

### Advantages

* Prevents starvation
* Balances short and long processes
* Improves fairness

### Disadvantages

* Requires frequent response ratio calculations

---

## 4. Lottery Scheduling

Processes are assigned lottery tickets and CPU allocation is determined randomly.

### Advantages

* Probabilistic fairness
* Flexible resource allocation

### Disadvantages

* Execution order is not deterministic

---

## 5. Multi-Level Queue (MLQ)

Custom hybrid scheduler implemented in this project.

### Queue Structure

Foreground Queue:

* Round Robin

Background Queue:

* First Come First Serve (FCFS)

Processes exceeding a predefined Round Robin threshold are automatically demoted to the FCFS queue.

### Advantages

* Fast response for short jobs
* Reduced context-switch overhead
* Improved throughput

---

# Kernel Modifications

The following Nachos kernel components were modified:

## thread.h

Added:

* rrQuantaUsed
* inRRQueue

## thread.cc

Updated thread initialization and scheduling state tracking.

## scheduler.cc

Modified:

* ReadyToRun()
* FindNextToRun()

Implemented:

* Multiple Ready Queues
* Priority Scheduling Logic
* MLQ Scheduling Logic

## timer.cc

Added:

* Quantum-based preemption support

## alarm.cc

Enhanced timer interrupt handling for scheduling decisions.

---

# Workflow

1. User selects scheduling algorithm.
2. User enters process information.
3. GUI generates configuration files.
4. Nachos scheduler executes simulation.
5. Execution output is captured.
6. Performance metrics are calculated.
7. Gantt chart is generated.
8. Results are displayed in the dashboard.

---

# Sample Metrics

| Metric  | Description             |
| ------- | ----------------------- |
| CT      | Completion Time         |
| TAT     | Turnaround Time         |
| WT      | Waiting Time            |
| Avg TAT | Average Turnaround Time |
| Avg WT  | Average Waiting Time    |

---

# Learning Outcomes

Through this project, I gained practical experience in:

* Operating System Internals
* CPU Scheduling Algorithms
* Kernel-Level Programming
* C++ System Development
* Python GUI Development
* Performance Evaluation
* Process Scheduling Concepts

---

# Future Enhancements

* Multi-Level Feedback Queue (MLFQ)
* I/O Burst Simulation
* CSV Export Functionality
* Advanced Analytics Dashboard
* Additional Scheduling Algorithms

---

# Results

Successfully implemented and compared:

* Round Robin
* Preemptive Priority
* HRRN
* Lottery Scheduling
* Multi-Level Queue Scheduling

The project enables visualization of scheduling behavior and provides performance analysis through interactive graphical interfaces.

---

# Authors

**Sai Harshith Reddy Karconda**

---

# License

This project was developed for academic and educational purposes.
