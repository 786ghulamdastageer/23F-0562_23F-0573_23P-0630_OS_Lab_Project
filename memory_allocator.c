#include "common.h"

Bed beds[MAX_BEDS];
int bed_count = MAX_BEDS;
int allocation_strategy = 1;
PageTableEntry page_table[10];
int page_size = 2;

void init_beds() {
    int idx = 0;
    for (int i = 0; i < ICU_COUNT; i++) {
        beds[idx].bed_id = idx;
        beds[idx].bed_type = ICU;
        beds[idx].status = FREE;
        beds[idx].patient_id = -1;
        beds[idx].care_units = 2;
        beds[idx].start = idx * 2;
        beds[idx].end = idx * 2 + 1;
        idx++;
    }
    for (int i = 0; i < ISOLATION_COUNT; i++) {
        beds[idx].bed_id = idx;
        beds[idx].bed_type = ISOLATION;
        beds[idx].status = FREE;
        beds[idx].patient_id = -1;
        beds[idx].care_units = 2;
        beds[idx].start = idx * 2;
        beds[idx].end = idx * 2 + 1;
        idx++;
    }
    for (int i = 0; i < GENERAL_COUNT; i++) {
        beds[idx].bed_id = idx;
        beds[idx].bed_type = GENERAL;
        beds[idx].status = FREE;
        beds[idx].patient_id = -1;
        beds[idx].care_units = 1;
        beds[idx].start = idx;
        beds[idx].end = idx;
        idx++;
    }
    bed_count = idx;
}

void init_page_table() {
    for (int i = 0; i < 10; i++) {
        page_table[i].page_id = i;
        page_table[i].patient_id = -1;
        page_table[i].is_occupied = 0;
    }
}

void update_page_table(int patient_id, int required_care) {
    int pages_needed = (required_care + page_size - 1) / page_size;
    int allocated = 0;
    
    for (int i = 0; i < 10 && allocated < pages_needed; i++) {
        if (page_table[i].is_occupied == 0) {
            page_table[i].is_occupied = 1;
            page_table[i].patient_id = patient_id;
            allocated++;
        }
    }
    
    int wasted = (pages_needed * page_size) - required_care;
    if (wasted > 0) {
        FILE* log = fopen(MEM_LOG, "a");
        if (log) {
            fprintf(log, "Internal fragmentation for Patient %d: %d care units wasted\n", patient_id, wasted);
            fclose(log);
        }
    }
}

void report_internal_fragmentation() {
    int total_wasted = 0;
    for (int i = 0; i < 10; i++) {
        if (page_table[i].is_occupied) {
            total_wasted += page_size;
        }
    }
    FILE* log = fopen(MEM_LOG, "a");
    if (log) {
        fprintf(log, "Page Table Internal Fragmentation Report: %d bytes wasted\n", total_wasted);
        fclose(log);
    }
}

int best_fit_allocator(int required_care, int bed_type) {
    int best_idx = -1;
    int min_waste = 999999;
    
    for (int i = 0; i < bed_count; i++) {
        if (beds[i].bed_type == bed_type && beds[i].status == FREE) {
            int size = beds[i].end - beds[i].start + 1;
            if (size >= required_care && (size - required_care) < min_waste) {
                min_waste = size - required_care;
                best_idx = i;
            }
        }
    }
    return best_idx;
}

int first_fit_allocator(int required_care, int bed_type) {
    for (int i = 0; i < bed_count; i++) {
        if (beds[i].bed_type == bed_type && beds[i].status == FREE) {
            int size = beds[i].end - beds[i].start + 1;
            if (size >= required_care) return i;
        }
    }
    return -1;
}

int worst_fit_allocator(int required_care, int bed_type) {
    int worst_idx = -1;
    int max_waste = -1;
    
    for (int i = 0; i < bed_count; i++) {
        if (beds[i].bed_type == bed_type && beds[i].status == FREE) {
            int size = beds[i].end - beds[i].start + 1;
            if (size >= required_care && size > max_waste) {
                max_waste = size;
                worst_idx = i;
            }
        }
    }
    return worst_idx;
}

void free_bed(int bed_id) {
    for (int i = 0; i < bed_count; i++) {
        if (beds[i].bed_id == bed_id) {
            beds[i].status = FREE;
            beds[i].patient_id = -1;
            break;
        }
    }
    
    for (int i = 0; i < bed_count - 1; i++) {
        if (beds[i].status == FREE && beds[i + 1].status == FREE) {
            beds[i].end = beds[i + 1].end;
            for (int j = i + 1; j < bed_count - 1; j++) {
                beds[j] = beds[j + 1];
            }
            bed_count--;
            i--;
        }
    }
}

void print_ward_map() {
    FILE* log = fopen(MEM_LOG, "a");
    if (!log) return;
    fprintf(log, "\n========== WARD MAP ==========\n");
    for (int i = 0; i < bed_count; i++) {
        char* type = (beds[i].bed_type == ICU) ? "ICU" : (beds[i].bed_type == ISOLATION) ? "ISO" : "GEN";
        fprintf(log, "Bed %d [%s]: %s\n", beds[i].bed_id, type, beds[i].status == FREE ? "FREE" : "OCCUPIED");
    }
    fclose(log);
}

void log_fragmentation() {
    int total_free = 0;
    int largest_free = 0;
    
    for (int i = 0; i < bed_count; i++) {
        if (beds[i].status == FREE) {
            int size = beds[i].end - beds[i].start + 1;
            total_free += size;
            if (size > largest_free) largest_free = size;
        }
    }
    
    float frag = (total_free > 0) ? (1.0 - (float)largest_free / total_free) * 100.0 : 0;
    FILE* log = fopen(MEM_LOG, "a");
    if (log) {
        fprintf(log, "Free: %d, Largest: %d, Ext Fragmentation: %.2f%%\n", total_free, largest_free, frag);
        fclose(log);
    }
    
    report_internal_fragmentation();
}