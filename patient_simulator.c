#include "common.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: patient_simulator <patient_id> <bed_id> <priority>\n");
        return 1;
    }
    
    int patient_id = atoi(argv[1]);
    int bed_id = atoi(argv[2]);
    int priority = atoi(argv[3]);
    
    int treatment_time;
    if (priority <= 2) treatment_time = rand() % 11 + 5;
    else if (priority == 3) treatment_time = rand() % 8 + 3;
    else treatment_time = rand() % 7 + 2;
    
    printf("Patient %d: Arrived at bed %d\n", patient_id, bed_id);
    printf("Patient %d: Starting treatment (Priority: %d, Duration: %d sec)\n",
           patient_id, priority, treatment_time);
    
    sleep(treatment_time);
    
    printf("Patient %d: Treatment completed. Discharging...\n", patient_id);
    
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd != -1) {
        char buffer[100];
        sprintf(buffer, "%d|%d", patient_id, bed_id);
        write(fd, buffer, strlen(buffer) + 1);
        close(fd);
    }
    
    printf("Patient %d: Discharged successfully\n", patient_id);
    
    return 0;
}