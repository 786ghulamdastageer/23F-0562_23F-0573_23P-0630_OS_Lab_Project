#include "common.h"

void fcfs_scheduling(Patient patients[], int n) {
    FILE* log = fopen(SCHED_LOG, "a");
    if (!log) return;
    
    fprintf(log, "\n========== FCFS SCHEDULING ==========\n");
    fprintf(log, "Gantt Chart: ");
    
    Patient* temp = malloc(n * sizeof(Patient));
    for (int i = 0; i < n; i++) temp[i] = patients[i];
    
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp[j].arrival_time > temp[j + 1].arrival_time) {
                Patient t = temp[j]; temp[j] = temp[j + 1]; temp[j + 1] = t;
            }
        }
    }
    
    int time = 0;
    float total_wait = 0, total_turn = 0;
    
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrival_time) time = temp[i].arrival_time;
        temp[i].waiting_time = time - temp[i].arrival_time;
        temp[i].turnaround_time = temp[i].waiting_time + temp[i].burst_time;
        time += temp[i].burst_time;
        total_wait += temp[i].waiting_time;
        total_turn += temp[i].turnaround_time;
        fprintf(log, "P%d ", temp[i].patient_id);
    }
    
    fprintf(log, "\nAverage Waiting Time: %.2f\n", total_wait / n);
    fprintf(log, "Average Turnaround Time: %.2f\n", total_turn / n);
    fclose(log);
    free(temp);
}

void sjf_scheduling(Patient patients[], int n) {
    FILE* log = fopen(SCHED_LOG, "a");
    if (!log) return;
    
    fprintf(log, "\n========== SJF SCHEDULING ==========\n");
    fprintf(log, "Gantt Chart: ");
    
    Patient* temp = malloc(n * sizeof(Patient));
    for (int i = 0; i < n; i++) temp[i] = patients[i];
    
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp[j].burst_time > temp[j + 1].burst_time) {
                Patient t = temp[j]; temp[j] = temp[j + 1]; temp[j + 1] = t;
            }
        }
    }
    
    int time = 0;
    float total_wait = 0, total_turn = 0;
    
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrival_time) time = temp[i].arrival_time;
        temp[i].waiting_time = time - temp[i].arrival_time;
        temp[i].turnaround_time = temp[i].waiting_time + temp[i].burst_time;
        time += temp[i].burst_time;
        total_wait += temp[i].waiting_time;
        total_turn += temp[i].turnaround_time;
        fprintf(log, "P%d ", temp[i].patient_id);
    }
    
    fprintf(log, "\nAverage Waiting Time: %.2f\n", total_wait / n);
    fprintf(log, "Average Turnaround Time: %.2f\n", total_turn / n);
    fclose(log);
    free(temp);
}

void priority_scheduling(Patient patients[], int n) {
    FILE* log = fopen(SCHED_LOG, "a");
    if (!log) return;
    
    fprintf(log, "\n========== PRIORITY SCHEDULING ==========\n");
    fprintf(log, "Gantt Chart: ");
    
    Patient* temp = malloc(n * sizeof(Patient));
    for (int i = 0; i < n; i++) temp[i] = patients[i];
    
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp[j].priority > temp[j + 1].priority) {
                Patient t = temp[j]; temp[j] = temp[j + 1]; temp[j + 1] = t;
            }
        }
    }
    
    int time = 0;
    float total_wait = 0, total_turn = 0;
    
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrival_time) time = temp[i].arrival_time;
        temp[i].waiting_time = time - temp[i].arrival_time;
        temp[i].turnaround_time = temp[i].waiting_time + temp[i].burst_time;
        time += temp[i].burst_time;
        total_wait += temp[i].waiting_time;
        total_turn += temp[i].turnaround_time;
        fprintf(log, "P%d ", temp[i].patient_id);
    }
    
    fprintf(log, "\nAverage Waiting Time: %.2f\n", total_wait / n);
    fprintf(log, "Average Turnaround Time: %.2f\n", total_turn / n);
    fclose(log);
    free(temp);
}

void round_robin_scheduling(Patient patients[], int n, int tq) {
    FILE* log = fopen(SCHED_LOG, "a");
    if (!log) return;
    
    fprintf(log, "\n========== ROUND ROBIN SCHEDULING (TQ=%d) ==========\n", tq);
    
    int* remaining = malloc(n * sizeof(int));
    int* waiting = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) remaining[i] = patients[i].burst_time;
    
    int time = 0, done = 0;
    while (done < n) {
        int progress = 0;
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0) {
                progress = 1;
                if (remaining[i] <= tq) {
                    time += remaining[i];
                    waiting[i] = time - patients[i].burst_time;
                    remaining[i] = 0;
                    done++;
                } else {
                    time += tq;
                    remaining[i] -= tq;
                }
            }
        }
        if (!progress) break;
    }
    
    float total_wait = 0, total_turn = 0;
    for (int i = 0; i < n; i++) {
        total_wait += waiting[i];
        total_turn += waiting[i] + patients[i].burst_time;
    }
    
    fprintf(log, "Average Waiting Time: %.2f\n", total_wait / n);
    fprintf(log, "Average Turnaround Time: %.2f\n", total_turn / n);
    fclose(log);
    free(remaining);
    free(waiting);
}