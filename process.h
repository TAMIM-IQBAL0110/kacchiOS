/* process.h - Process manager for kacchiOS */
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Process states */
typedef enum {
    TERMINATED = 0,
    READY = 1,
    CURRENT = 2
} process_state_t;

/* CPU context for context switching */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
} cpu_context_t;

/* Process Control Block (PCB) */
typedef struct {
    uint32_t process_id;
    process_state_t state;
    uint32_t priority;
    uint32_t stack_base;
    uint32_t stack_size;
    uint32_t heap_base;
    uint32_t heap_size;
    cpu_context_t context;
    uint32_t creation_time;
    uint32_t wait_time;         /* For aging */
} process_control_block_t;

/* Process table */
typedef struct {
    process_control_block_t processes[256];
    uint32_t process_count;
    uint32_t next_process_id;
} process_table_t;

/* Function declarations */
void process_init(void);
uint32_t process_create(uint32_t priority, uint32_t stack_size, uint32_t heap_size);
void process_terminate(uint32_t process_id);
void process_set_state(uint32_t process_id, process_state_t state);
process_state_t process_get_state(uint32_t process_id);
process_control_block_t* process_get_pcb(uint32_t process_id);
void process_print_table(void);

#endif
