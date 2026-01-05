/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

#define MAX_INPUT 128

void kmain(void) {
    char input[MAX_INPUT];
    int pos = 0;
    
    /* Initialize hardware */
    serial_init();
    memory_init();
    process_init();
    scheduler_init(RR, 5); /* Round Robin, 5ms quantum */
    
    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Running null process...\n\n");

    /* Create 10 demo processes with varying priorities */
    uint32_t pids[10];
    for (uint32_t i = 0; i < 10; i++) {
        uint32_t prio = (i % 4) + 1;      /* small spread of priorities */
        pids[i] = process_create(prio, 4096, 8192);
    }

    serial_puts("Created 10 processes: ");
    for (uint32_t i = 0; i < 10; i++) {
        serial_put_dec(pids[i]);
        if (i != 9) {
            serial_puts(", ");
        }
    }
    serial_puts("\n");

    /* Run a few scheduler ticks to show rotation */
    for (int tick = 0; tick < 12; tick++) {
        scheduler_update_time();
        scheduler_schedule();

        /* Find which PCB is marked CURRENT */
        uint32_t current_pid = 0;
        for (uint32_t pid = 0; pid < 20; pid++) {
            process_control_block_t *pcb = process_get_pcb(pid);
            if (pcb != 0 && pcb->state == CURRENT) {
                current_pid = pid;
                break;
            }
        }

        serial_puts("[tick ");
        serial_put_dec(tick);
        serial_puts("] current PID: ");
        serial_put_dec(current_pid);
        serial_puts("\n");
    }

    /* Print the process table so users can see allocations */
    process_print_table();
    
    /* Main loop - the "null process" with command interface */
    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;
        
        /* Read input line */
        while (1) {
            char c = serial_getc();
            
            /* Handle Enter key */
            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");  /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);  /* Echo character */
            }
        }
        
        /* Process commands */
        if (pos > 0) {
            if (strcmp(input, "ps") == 0) {
                /* Show process table */
                process_print_table();
            }
            else if (strcmp(input, "mem") == 0) {
                /* Show memory status */
                memory_print_status();
            }
            else if (strcmp(input, "sched") == 0) {
                /* Show scheduler status and trigger a few ticks */
                scheduler_print_status();
                serial_puts("Running 5 scheduler ticks...\n");
                for (int i = 0; i < 5; i++) {
                    scheduler_update_time();
                    scheduler_schedule();
                    
                    uint32_t current_pid = 0;
                    for (uint32_t pid = 0; pid < 20; pid++) {
                        process_control_block_t *pcb = process_get_pcb(pid);
                        if (pcb != 0 && pcb->state == CURRENT) {
                            current_pid = pid;
                            break;
                        }
                    }
                    
                    serial_puts("[tick ");
                    serial_put_dec(i);
                    serial_puts("] current PID: ");
                    serial_put_dec(current_pid);
                    serial_puts("\n");
                }
            }
            else if (strcmp(input, "help") == 0) {
                /* Show available commands */
                serial_puts("\n=== kacchiOS Commands ===\n");
                serial_puts("ps      - Show process table\n");
                serial_puts("mem     - Show memory status\n");
                serial_puts("sched   - Show scheduler status & run ticks\n");
                serial_puts("create  - Create a new process\n");
                serial_puts("help    - Show this help message\n\n");
            }
            else if (strcmp(input, "create") == 0) {
                /* Create a new process */
                uint32_t new_pid = process_create(2, 4096, 8192);
                serial_puts("Created new process with PID: ");
                serial_put_dec(new_pid);
                serial_puts("\n");
                process_print_table();
            }
            else {
                serial_puts("Unknown command: ");
                serial_puts(input);
                serial_puts("\nType 'help' for available commands.\n");
            }
        }
    }
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}