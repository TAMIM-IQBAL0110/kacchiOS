/* test_kernel.c - Kernel entry point for running tests */
#include "types.h"
#include "serial.h"
#include "test_suite.h"

void kmain(void) {
    /* Initialize hardware */
    serial_init();
    
    /* Run all tests */
    run_all_tests();
    
    /* Halt the system */
    serial_puts("\n\nTests completed. Halting system...\n");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
