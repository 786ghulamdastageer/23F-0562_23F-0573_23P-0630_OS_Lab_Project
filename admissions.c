#include "common.h"

PriorityQueue pq;
pthread_mutex_t bed_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bed_freed = PTHREAD_COND_INITIALIZER;
sem_t icu_sem;
sem_t isolation_sem;
int shutdown_flag = 0;
int total_patients = 0;
Patient all_patients[100];
int shm_id;
Bed* shared_beds;

void init_pq(PriorityQueue* q) {
    q->front = NULL;
    q->count = 0;
}

void pq_enqueue(PriorityQueue* q, Patient p) {
    PQNode* new_node = (PQNode*)malloc(sizeof(PQNode));
    new_node->p = p;
    new_node->next = NULL;
    
    if (q->front == NULL || p.priority < q->front->p.priority) {
        new_node->next = q->front;
        q->front = new_node;
    } else {
        PQNode* curr = q->front;
        while (curr->next != NULL && curr->next->p.priority <= p.priority) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
    q->count++;
}

Patient pq_dequeue(PriorityQueue* q) {
    Patient empty = { -1, "", 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0 };
    if (q->front == NULL) return empty;
    
    PQNode* temp = q->front;
    Patient p = temp->p;
    q->front = q->front->next;
    free(temp);
    q->count--;
    return p;
}

int is_pq_empty(PriorityQueue* q) {
    return q->front == NULL;
}

void print_patient_table() {
    printf("\n================================================================================\n");
    printf("PATIENT SUMMARY TABLE\n");
    printf("================================================================================\n");
    printf("ID\tName\t\tAge\tSeverity\tPriority\tBed\tBurst\tWait\tTurnaround\n");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < total_patients; i++) {
        Patient p = all_patients[i];
        printf("%d\t%-12s\t%d\t%d\t\t%d\t\t%d\t%d\t%d\t%d\n",
               p.patient_id, p.name, p.age, p.severity, p.priority,
               p.assigned_bed, p.burst_time, p.waiting_time, p.turnaround_time);
    }
    printf("================================================================================\n");
}

void calculate_averages() {
    float avg_wait = 0, avg_turn = 0;
    for (int i = 0; i < total_patients; i++) {
        avg_wait += all_patients[i].waiting_time;
        avg_turn += all_patients[i].turnaround_time;
    }
    if (total_patients > 0) {
        avg_wait /= total_patients;
        avg_turn /= total_patients;
    }
    printf("\n========== OVERALL STATISTICS ==========\n");
    printf("Total Patients: %d\n", total_patients);
    printf("Average Waiting Time: %.2f\n", avg_wait);
    printf("Average Turnaround Time: %.2f\n", avg_turn);
    printf("========================================\n");
}

void* receptionist_thread(void* arg) {
    (void)arg;
    char buffer[256];
    
    while (!shutdown_flag) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }
        
        Patient p;
        sscanf(buffer, "%d|%[^|]|%d|%d|%d",
               &p.patient_id, p.name, &p.age, &p.severity, &p.priority);
        
        p.required_care = (p.priority <= 2) ? 2 : (p.priority == 3) ? 2 : 1;
        p.arrival_time = time(NULL);
        p.burst_time = (p.priority <= 2) ? (rand() % 11 + 5) : (p.priority == 3) ? (rand() % 8 + 3) : (rand() % 7 + 2);
        
        pthread_mutex_lock(&pq_mutex);
        pq_enqueue(&pq, p);
        pthread_mutex_unlock(&pq_mutex);
    }
    return NULL;
}

void* nurse_icu_thread(void* arg) {
    (void)arg;
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) return NULL;
    
    char buffer[100];
    while (!shutdown_flag) {
        if (read(fd, buffer, sizeof(buffer)) > 0) {
            int patient_id, bed_id;
            sscanf(buffer, "%d|%d", &patient_id, &bed_id);
            
            pthread_mutex_lock(&bed_mutex);
            if (shared_beds[bed_id].bed_type == ICU) {
                free_bed(bed_id);
            }
            pthread_cond_broadcast(&bed_freed);
            pthread_mutex_unlock(&bed_mutex);
            
            for (int i = 0; i < bed_count; i++) {
                if (shared_beds[i].bed_id == bed_id && shared_beds[i].bed_type == ICU) {
                    sem_post(&icu_sem);
                    break;
                }
            }
        }
    }
    close(fd);
    return NULL;
}

void* nurse_isolation_thread(void* arg) {
    (void)arg;
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) return NULL;
    
    char buffer[100];
    while (!shutdown_flag) {
        if (read(fd, buffer, sizeof(buffer)) > 0) {
            int patient_id, bed_id;
            sscanf(buffer, "%d|%d", &patient_id, &bed_id);
            
            pthread_mutex_lock(&bed_mutex);
            if (shared_beds[bed_id].bed_type == ISOLATION) {
                free_bed(bed_id);
            }
            pthread_cond_broadcast(&bed_freed);
            pthread_mutex_unlock(&bed_mutex);
            
            for (int i = 0; i < bed_count; i++) {
                if (shared_beds[i].bed_id == bed_id && shared_beds[i].bed_type == ISOLATION) {
                    sem_post(&isolation_sem);
                    break;
                }
            }
        }
    }
    close(fd);
    return NULL;
}

void* nurse_general_thread(void* arg) {
    (void)arg;
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) return NULL;
    
    char buffer[100];
    while (!shutdown_flag) {
        if (read(fd, buffer, sizeof(buffer)) > 0) {
            int patient_id, bed_id;
            sscanf(buffer, "%d|%d", &patient_id, &bed_id);
            
            pthread_mutex_lock(&bed_mutex);
            if (shared_beds[bed_id].bed_type == GENERAL) {
                free_bed(bed_id);
            }
            pthread_cond_broadcast(&bed_freed);
            pthread_mutex_unlock(&bed_mutex);
        }
    }
    close(fd);
    return NULL;
}

void* scheduler_thread(void* arg) {
    (void)arg;
    
    while (!shutdown_flag) {
        pthread_mutex_lock(&pq_mutex);
        if (is_pq_empty(&pq)) {
            pthread_mutex_unlock(&pq_mutex);
            usleep(100000);
            continue;
        }
        Patient p = pq_dequeue(&pq);
        pthread_mutex_unlock(&pq_mutex);
        
        int bed_type = (p.priority <= 2) ? ICU : (p.priority == 3) ? ISOLATION : GENERAL;
        
        if (bed_type == ICU) sem_wait(&icu_sem);
        if (bed_type == ISOLATION) sem_wait(&isolation_sem);
        
        pthread_mutex_lock(&bed_mutex);
        
        int bed_idx;
        if (allocation_strategy == 1) bed_idx = best_fit_allocator(p.required_care, bed_type);
        else if (allocation_strategy == 2) bed_idx = first_fit_allocator(p.required_care, bed_type);
        else bed_idx = worst_fit_allocator(p.required_care, bed_type);
        
        while (bed_idx == -1 && !shutdown_flag) {
            pthread_cond_wait(&bed_freed, &bed_mutex);
            if (allocation_strategy == 1) bed_idx = best_fit_allocator(p.required_care, bed_type);
            else if (allocation_strategy == 2) bed_idx = first_fit_allocator(p.required_care, bed_type);
            else bed_idx = worst_fit_allocator(p.required_care, bed_type);
        }
        
        if (bed_idx != -1) {
            shared_beds[bed_idx].status = OCCUPIED;
            shared_beds[bed_idx].patient_id = p.patient_id;
            p.assigned_bed = shared_beds[bed_idx].bed_id;
            p.start_time = time(NULL);
            
            p.waiting_time = p.start_time - p.arrival_time;
            p.turnaround_time = p.waiting_time + p.burst_time;
            
            all_patients[total_patients++] = p;
            update_page_table(p.patient_id, p.required_care);
            
            pid_t pid = fork();
            if (pid == 0) {
                char pid_str[10], bed_str[10], prio_str[10];
                sprintf(pid_str, "%d", p.patient_id);
                sprintf(bed_str, "%d", p.assigned_bed);
                sprintf(prio_str, "%d", p.priority);
                execl("./src/patient_simulator", "patient_simulator", pid_str, bed_str, prio_str, NULL);
                exit(0);
            }
            
            print_ward_map();
            log_fragmentation();
        }
        
        pthread_mutex_unlock(&bed_mutex);
    }
    return NULL;
}

void* nurse_thread(void* arg) {
    (void)arg;
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) return NULL;
    
    char buffer[100];
    while (!shutdown_flag) {
        if (read(fd, buffer, sizeof(buffer)) > 0) {
            int patient_id, bed_id;
            sscanf(buffer, "%d|%d", &patient_id, &bed_id);
            
            pthread_mutex_lock(&bed_mutex);
            free_bed(bed_id);
            print_ward_map();
            log_fragmentation();
            pthread_cond_broadcast(&bed_freed);
            pthread_mutex_unlock(&bed_mutex);
            
            for (int i = 0; i < bed_count; i++) {
                if (shared_beds[i].bed_id == bed_id) {
                    if (shared_beds[i].bed_type == ICU) sem_post(&icu_sem);
                    if (shared_beds[i].bed_type == ISOLATION) sem_post(&isolation_sem);
                    break;
                }
            }
        }
    }
    close(fd);
    return NULL;
}

void handle_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigterm(int sig) {
    (void)sig;
    shutdown_flag = 1;
    
    sem_destroy(&icu_sem);
    sem_destroy(&isolation_sem);
    pthread_mutex_destroy(&bed_mutex);
    pthread_mutex_destroy(&pq_mutex);
    pthread_cond_destroy(&bed_freed);
    
    if (shm_id != -1) {
        shmdt(shared_beds);
        shmctl(shm_id, IPC_RMID, NULL);
    }
    
    unlink(FIFO_PATH);
    
    print_patient_table();
    calculate_averages();
    
    if (total_patients > 0) {
        fcfs_scheduling(all_patients, total_patients);
        sjf_scheduling(all_patients, total_patients);
        priority_scheduling(all_patients, total_patients);
        round_robin_scheduling(all_patients, total_patients, 10);
    }
    
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGCHLD, handle_sigchld);
    signal(SIGTERM, handle_sigterm);
    
    if (argc > 1 && strcmp(argv[1], "--strategy") == 0 && argc > 2) {
        if (strcmp(argv[2], "first") == 0) allocation_strategy = 2;
        else if (strcmp(argv[2], "worst") == 0) allocation_strategy = 3;
        else allocation_strategy = 1;
    }
    
    printf("Allocation Strategy: %s\n", 
           allocation_strategy == 1 ? "Best-Fit" : (allocation_strategy == 2 ? "First-Fit" : "Worst-Fit"));
    
    shm_id = shmget(SHM_KEY, sizeof(Bed) * MAX_BEDS, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }
    
    shared_beds = (Bed*)shmat(shm_id, NULL, 0);
    if (shared_beds == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    init_beds();
    memcpy(shared_beds, beds, sizeof(Bed) * MAX_BEDS);
    
    init_pq(&pq);
    init_page_table();
    
    sem_init(&icu_sem, 0, ICU_COUNT);
    sem_init(&isolation_sem, 0, ISOLATION_COUNT);
    
    pthread_t rec_thread, sched_thread, nurse_icu, nurse_iso, nurse_gen;
    pthread_create(&rec_thread, NULL, receptionist_thread, NULL);
    pthread_create(&sched_thread, NULL, scheduler_thread, NULL);
    pthread_create(&nurse_icu, NULL, nurse_icu_thread, NULL);
    pthread_create(&nurse_iso, NULL, nurse_isolation_thread, NULL);
    pthread_create(&nurse_gen, NULL, nurse_general_thread, NULL);
    
    while (!shutdown_flag) {
        sleep(1);
    }
    
    pthread_join(rec_thread, NULL);
    pthread_join(sched_thread, NULL);
    pthread_join(nurse_icu, NULL);
    pthread_join(nurse_iso, NULL);
    pthread_join(nurse_gen, NULL);
    
    return 0;
}