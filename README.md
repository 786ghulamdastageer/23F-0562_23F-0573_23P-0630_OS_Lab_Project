# 🏥 Hospital Patient Triage & Bed Allocator

## 📌 Project Overview

This is a complete operating system simulation project that models a hospital emergency room system. Patients arrive, are triaged (assigned priority levels 1-5), and allocated to appropriate beds (ICU, Isolation, or General Ward). The system demonstrates core OS concepts including process management, inter-process communication (IPC), multithreading, synchronization, CPU scheduling, and memory management.

## 🎯 OS Concepts Implemented

| Concept | Implementation |
|---------|----------------|
| Process Management | `fork()` + `execv()` for patient admission, `SIGCHLD` handler, `waitpid()` |
| IPC - Pipes | Anonymous pipe for triage data, Named FIFO for discharge notification |
| IPC - Shared Memory | Bed bitmap stored in shared memory segment (`shmget`/`shmat`) |
| CPU Scheduling | Priority queue with 4 algorithms (FCFS, SJF, Priority, RR) |
| Multithreading | 5 POSIX threads (Receptionist, Scheduler, ICU Nurse, Isolation Nurse, General Nurse) |
| Synchronization | Mutex locks, condition variables, counting semaphores |
| Memory Management | Best-Fit/First-Fit/Worst-Fit allocation, coalescing, fragmentation reporting, paging simulation |

## 📁 Project Structure
