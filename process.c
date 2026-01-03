/* process.c - Process manager implementation for kacchiOS */
#include "process.h"
#include "memory.h"
#include "serial.h"
#include "string.h"

static process_table_t process_table;
static uint32_t global_time = 0;

/**
 * Initialize the process manager
 */
void process_init(void) {
    process_table.process_count = 0;
    process_table.next_process_id = 1;
    
    /* Create the idle/null process */
    process_table.processes[0].process_id = 0;
    process_table.processes[0].state = CURRENT;
    process_table.processes[0].priority = 0;
    process_table.processes[0].stack_base = 0x20000;
    process_table.processes[0].stack_size = 0x1000;
    process_table.processes[0].heap_base = 0x21000;
    process_table.processes[0].heap_size = 0x2000;
    process_table.processes[0].creation_time = 0;
    process_table.processes[0].wait_time = 0;
    
    process_table.process_count = 1;
    
    serial_puts("[PROCESS] Process manager initialized\n");
}

/**
 * Create a new process
 * @param priority: Process priority (0-255, lower number = higher priority)
 * @param stack_size: Size of stack to allocate
 * @param heap_size: Size of heap to allocate
 * @return: Process ID, or 0 on failure
 */
uint32_t process_create(uint32_t priority, uint32_t stack_size, uint32_t heap_size) {
    if (process_table.process_count >= 256) {
        serial_puts("[PROCESS] ERROR: Process table full\n");
        return 0;
    }
    
    uint32_t pid = process_table.next_process_id++;
    process_control_block_t *pcb = &process_table.processes[process_table.process_count];
    
    /* Allocate stack and heap */
    uint32_t stack_base = memory_allocate(stack_size, pid);
    uint32_t heap_base = memory_allocate(heap_size, pid);
    
    if (stack_base == 0 || heap_base == 0) {
        serial_puts("[PROCESS] ERROR: Failed to allocate memory for process\n");
        return 0;
    }
    
    /* Initialize PCB */
    pcb->process_id = pid;
    pcb->state = READY;
    pcb->priority = priority;
    pcb->stack_base = stack_base;
    pcb->stack_size = stack_size;
    pcb->heap_base = heap_base;
    pcb->heap_size = heap_size;
    pcb->creation_time = global_time;
    pcb->wait_time = 0;
    
    /* Initialize CPU context */
    pcb->context.esp = stack_base + stack_size;
    pcb->context.ebp = pcb->context.esp;
    pcb->context.eip = 0;
    
    process_table.process_count++;
    
    return pid;
}

/**
 * Terminate a process
 * @param process_id: ID of process to terminate
 */
void process_terminate(uint32_t process_id) {
    uint32_t i;
    
    for (i = 0; i < process_table.process_count; i++) {
        if (process_table.processes[i].process_id == process_id) {
            process_table.processes[i].state = TERMINATED;
            
            /* Free memory allocated to this process */
            memory_free_process(process_id);
            
            serial_puts("[PROCESS] Process ");
            serial_put_dec(process_id);
            serial_puts(" terminated\n");
            
            return;
        }
    }
    
    serial_puts("[PROCESS] WARNING: Process not found\n");
}

/**
 * Set the state of a process
 * @param process_id: ID of process
 * @param state: New state
 */
void process_set_state(uint32_t process_id, process_state_t state) {
    uint32_t i;
    
    for (i = 0; i < process_table.process_count; i++) {
        if (process_table.processes[i].process_id == process_id) {
            process_table.processes[i].state = state;
            return;
        }
    }
}

/**
 * Get the state of a process
 * @param process_id: ID of process
 * @return: Current state, or TERMINATED if not found
 */
process_state_t process_get_state(uint32_t process_id) {
    uint32_t i;
    
    for (i = 0; i < process_table.process_count; i++) {
        if (process_table.processes[i].process_id == process_id) {
            return process_table.processes[i].state;
        }
    }
    
    return TERMINATED;
}

/**
 * Get PCB of a process
 * @param process_id: ID of process
 * @return: Pointer to PCB, or NULL if not found
 */
process_control_block_t* process_get_pcb(uint32_t process_id) {
    uint32_t i;
    
    for (i = 0; i < process_table.process_count; i++) {
        if (process_table.processes[i].process_id == process_id) {
            return &process_table.processes[i];
        }
    }
    
    return NULL;
}

/**
 * Print process table
 */
void process_print_table(void) {
    uint32_t i;
    
    serial_puts("\n=== Process Table ===\n");
    serial_puts("PID | State    | Priority | Stack Base | Heap Base | Wait Time\n");
    serial_puts("-----------------------------------------------------------\n");
    
    for (i = 0; i < process_table.process_count; i++) {
        process_control_block_t *pcb = &process_table.processes[i];
        
        serial_put_dec(pcb->process_id);
        serial_puts("   | ");
        
        if (pcb->state == CURRENT) {
            serial_puts("CURRENT ");
        } else if (pcb->state == READY) {
            serial_puts("READY   ");
        } else {
            serial_puts("TERM.   ");
        }
        
        serial_puts("| ");
        serial_put_dec(pcb->priority);
        serial_puts("       | 0x");
        serial_put_hex(pcb->stack_base);
        serial_puts(" | 0x");
        serial_put_hex(pcb->heap_base);
        serial_puts(" | ");
        serial_put_dec(pcb->wait_time);
        serial_puts("\n");
    }
    
    serial_puts("-----------------------------------------------------------\n\n");
}
