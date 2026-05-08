#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define MAX_BEDS 20
#define ICU_COUNT 4
#define ISOLATION_COUNT 4
#define GENERAL_COUNT 12
#define SHM_KEY 0xBEDF00D
#define FIFO_PATH "/tmp/discharge_fifo"
#define SCHED_LOG "logs/schedule_log.txt"
#define MEM_LOG "logs/memory_log.txt"

enum BedType { ICU = 1, ISOLATION = 2, GENERAL = 3 };
enum BedStatus { FREE = 0, OCCUPIED = 1 };

typedef struct {
    int bed_id;
    int bed_type;
    int status;
    int patient_id;
    int care_units;
    int start;
    int end;
} Bed;

typedef struct {
    int patient_id;
    char name[50];
    int age;
    int severity;
    int priority;
    int required_care;
    int assigned_bed;
    int arrival_time;
    int start_time;
    int discharge_time;
    int burst_time;
    int waiting_time;
    int turnaround_time;
} Patient;

typedef struct PQNode {
    Patient p;
    struct PQNode* next;
} PQNode;

typedef struct {
    PQNode* front;
    int count;
} PriorityQueue;

typedef struct {
    int page_id;
    int patient_id;
    int is_occupied;
} PageTableEntry;

extern Bed* beds;
extern int bed_count;
extern PriorityQueue pq;
extern pthread_mutex_t bed_mutex;
extern pthread_mutex_t pq_mutex;
extern pthread_cond_t bed_freed;
extern sem_t icu_sem;
extern sem_t isolation_sem;
extern int shutdown_flag;
extern int allocation_strategy;
extern int total_patients;
extern Patient all_patients[100];
extern PageTableEntry page_table[10];
extern int page_size;

void init_beds();
void init_pq(PriorityQueue* q);
void pq_enqueue(PriorityQueue* q, Patient p);
Patient pq_dequeue(PriorityQueue* q);
int is_pq_empty(PriorityQueue* q);

int best_fit_allocator(int required_care, int bed_type);
int first_fit_allocator(int required_care, int bed_type);
int worst_fit_allocator(int required_care, int bed_type);
void free_bed(int bed_id);
void coalesce_free_partitions();
void print_ward_map();
void log_fragmentation();

void fcfs_scheduling(Patient patients[], int n);
void sjf_scheduling(Patient patients[], int n);
void priority_scheduling(Patient patients[], int n);
void round_robin_scheduling(Patient patients[], int n, int tq);

void print_patient_table();
void calculate_averages();
void init_page_table();
void update_page_table(int patient_id, int required_care);
void report_internal_fragmentation();

void handle_sigchld(int sig);
void handle_sigterm(int sig);

#endif