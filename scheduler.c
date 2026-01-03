/* scheduler.c - Process scheduler implementation for kacchiOS */
#include "scheduler.h"
#include "process.h"
#include "memory.h"
#include "serial.h"
#include "string.h"
static scheduler_t scheduler;
static uint32_t current_process_id = 0;
static uint32_t time_since_switch = 0;
/**
 * Initialize the scheduler
 * @param algorithm: Scheduling algorithm (FCFS or RR)
 * @param time_quantum: Time quantum for round robin (ms)
 */
void scheduler_init(scheduling_algorithm_t algorithm, uint32_t time_quantum) {
    scheduler.algorithm = algorithm;
    scheduler.time_quantum = time_quantum;
    scheduler.current_time = 0;
    scheduler.process_count = 1;  /* Null process */
    current_process_id = 0;
    time_since_switch = 0;
    serial_puts("[SCHEDULER] Scheduler initialized with ");
    if (algorithm == FCFS) {
        serial_puts("FCFS algorithm\n");
    } 
    else {
        serial_puts("Round Robin algorithm (");
        serial_put_dec(time_quantum);
        serial_puts("ms)\n");
    }
}

/**
 * Get the next process to run
 * Uses the configured scheduling algorithm
 * @return: Process ID of next process to run
 */
uint32_t scheduler_get_next_process(void) {
    uint32_t i;
    uint32_t next_pid = 0;
    uint32_t highest_priority = 256;
    uint32_t lowest_wait_time = 0xFFFFFFFF;
    uint32_t found = 0;
    
    if (scheduler.algorithm == FCFS) {
        /* First Come First Served - pick first READY process with highest priority */
        for (i = 1; i < 256; i++) {
            process_control_block_t *pcb = process_get_pcb(i);
            
            if (pcb != NULL && pcb->state == READY) {
                if (!found || pcb->priority < highest_priority || 
                    (pcb->priority == highest_priority && i < next_pid)) {
                    highest_priority = pcb->priority;
                    next_pid = i;
                    found = 1;
                }
            }
        }
    } else {
        /* Round Robin with Aging */
        /* First, check if time quantum expired for current process */
        if (time_since_switch >= scheduler.time_quantum) {
            process_control_block_t *current = process_get_pcb(current_process_id);
            if (current != NULL && current->state == CURRENT) {
                current->state = READY;
            }
        }
        /* Find READY process with lowest wait time (aging) */
        for (i = 1; i < 256; i++) {
            process_control_block_t *pcb = process_get_pcb(i);
            if (pcb != NULL && pcb->state == READY) {
                if (!found || pcb->wait_time < lowest_wait_time || 
                    (pcb->wait_time == lowest_wait_time && pcb->priority < highest_priority) ||
                    (pcb->wait_time == lowest_wait_time && pcb->priority == highest_priority && i < next_pid)) {
                    lowest_wait_time = pcb->wait_time;
                    highest_priority = pcb->priority;
                    next_pid = i;
                    found = 1;
                }
            }
        }
    }
    /* If no READY process, return idle process (PID 0) */
    if (!found) {
        next_pid = 0;
    }
    return next_pid;
}
/**
 * Perform a context switch
 * @param from_pid: Current process ID
 * @param to_pid: Next process ID
 */
void scheduler_context_switch(uint32_t from_pid, uint32_t to_pid) {
    process_control_block_t *from = process_get_pcb(from_pid);
    process_control_block_t *to = process_get_pcb(to_pid);
    if (from != NULL && from->state == CURRENT) {
        from->state = READY;
    }
    if (to != NULL) {
        to->state = CURRENT;
        current_process_id = to_pid;
        time_since_switch = 0;
    }
}
//Schedule and perform context switch
void scheduler_schedule(void) {
    uint32_t next_pid = scheduler_get_next_process();
    if (next_pid != current_process_id) {
        scheduler_context_switch(current_process_id, next_pid);
    }
}
 //Update scheduler time (called periodically)
void scheduler_update_time(void) {
    uint32_t i;
    scheduler.current_time++;
    time_since_switch++;
    //Update wait times for aging
    for (i = 0; i < 256; i++) {
        process_control_block_t *pcb = process_get_pcb(i);
        if (pcb != NULL && pcb->state == READY) {
            pcb->wait_time++;
        }
    }
    //Trigger scheduling decision if time quantum expired */
    if (scheduler.algorithm == RR && time_since_switch >= scheduler.time_quantum) {
        scheduler_schedule();
    }
}

//Apply aging to waiting processes
//Increases priority of processes waiting too long
void scheduler_apply_aging(void) {
    uint32_t i;
    const uint32_t AGING_THRESHOLD = 1000;  /* Milliseconds */
    for (i = 0; i < 256; i++) {
        process_control_block_t *pcb = process_get_pcb(i);
        
        if (pcb != NULL && pcb->state == READY && pcb->wait_time > AGING_THRESHOLD) {
            /* Increase priority (decrease priority value) */
            if (pcb->priority > 0) {
                pcb->priority--;
            }
            pcb->wait_time = 0;  /* Reset wait time */
        }
    }
}

//Print scheduler status
void scheduler_print_status(void) {
    serial_puts("\n=== Scheduler Status ===\n");
    serial_puts("Algorithm: ");
    if (scheduler.algorithm == FCFS) {
        serial_puts("FCFS\n");
    } else {
        serial_puts("Round Robin (");
        serial_put_dec(scheduler.time_quantum);
        serial_puts("ms)\n");
    }
    serial_puts("Current Time: ");
    serial_put_dec(scheduler.current_time);
    serial_puts("ms\n");
    serial_puts("Current Process: ");
    serial_put_dec(current_process_id);
    serial_puts("\n");
    serial_puts("Time Since Switch: ");
    serial_put_dec(time_since_switch);
    serial_puts("ms\n\n");
}
