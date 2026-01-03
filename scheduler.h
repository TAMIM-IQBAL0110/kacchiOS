/* scheduler.h - Process scheduler for kacchiOS */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "process.h"

/* Scheduling algorithm types */
typedef enum {
    FCFS = 0,           /* First Come First Served */
    RR = 1              /* Round Robin */
} scheduling_algorithm_t;

/* Scheduler structure */
typedef struct {
    scheduling_algorithm_t algorithm;
    uint32_t time_quantum;      /* Time slice for round robin (ms) */
    uint32_t current_time;
    uint32_t process_count;
} scheduler_t;

/* Function declarations */
void scheduler_init(scheduling_algorithm_t algorithm, uint32_t time_quantum);
void scheduler_schedule(void);
void scheduler_context_switch(uint32_t from_pid, uint32_t to_pid);
uint32_t scheduler_get_next_process(void);
void scheduler_update_time(void);
void scheduler_apply_aging(void);
void scheduler_print_status(void);

#endif
