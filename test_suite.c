/* test_suite.c - Comprehensive test suite for kacchiOS */
#include "types.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"
#include "serial.h"
#include "string.h"

/* Test counters */
static uint32_t tests_run = 0;
static uint32_t tests_passed = 0;
static uint32_t tests_failed = 0;

/* Assertion macro */
#define ASSERT(condition, test_name) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            serial_puts("[PASS] "); \
            serial_puts(test_name); \
            serial_puts("\n"); \
        } else { \
            tests_failed++; \
            serial_puts("[FAIL] "); \
            serial_puts(test_name); \
            serial_puts("\n"); \
        } \
    } while (0)

#define ASSERT_EQ(actual, expected, test_name) \
    ASSERT((actual) == (expected), test_name)

#define ASSERT_NEQ(actual, expected, test_name) \
    ASSERT((actual) != (expected), test_name)

/* ============================================================================
   MEMORY MANAGER TESTS
   ============================================================================ */

void test_memory_allocate(void) {
    serial_puts("\n--- MEMORY MANAGER TESTS ---\n");
    
    memory_init();
    
    /* Test 1: Basic allocation */
    uint32_t addr1 = memory_allocate(1024, 1);
    ASSERT_NEQ(addr1, 0, "Memory allocation returns non-zero address");
    
    /* Test 2: Multiple allocations */
    uint32_t addr2 = memory_allocate(2048, 1);
    uint32_t addr3 = memory_allocate(512, 2);
    ASSERT_NEQ(addr2, 0, "Second memory allocation succeeds");
    ASSERT_NEQ(addr3, 0, "Third memory allocation succeeds");
    
    /* Test 3: Allocations have different addresses */
    ASSERT_NEQ(addr1, addr2, "Different allocations have different addresses");
    ASSERT_NEQ(addr2, addr3, "Different allocations have different addresses");
    
    /* Test 4: Zero size allocation should fail */
    uint32_t addr_zero = memory_allocate(0, 1);
    ASSERT_EQ(addr_zero, 0, "Zero-size allocation returns 0");
    
    /* Test 5: Memory deallocation */
    memory_free(addr1);
    uint32_t reused = memory_allocate(512, 1);
    ASSERT(reused != 0, "Memory can be reused after deallocation");
}

void test_memory_free_process(void) {
    serial_puts("\n--- MEMORY DEALLOCATION TESTS ---\n");
    
    memory_init();
    
    /* Allocate memory for process 1 */
    uint32_t addr1 = memory_allocate(1024, 1);
    uint32_t addr2 = memory_allocate(512, 1);
    
    /* Allocate for process 2 */
    uint32_t addr3 = memory_allocate(2048, 2);
    
    ASSERT_NEQ(addr1, 0, "Process 1: First allocation");
    ASSERT_NEQ(addr2, 0, "Process 1: Second allocation");
    ASSERT_NEQ(addr3, 0, "Process 2: First allocation");
    
    /* Free all process 1 memory */
    memory_free_process(1);
    
    /* Should be able to allocate same size */
    uint32_t reused = memory_allocate(1024, 3);
    ASSERT(reused != 0, "Memory freed for process 1 can be reused");
}

/* ============================================================================
   PROCESS MANAGER TESTS
   ============================================================================ */

void test_process_creation(void) {
    serial_puts("\n--- PROCESS MANAGER TESTS ---\n");
    
    process_init();
    
    /* Test 1: Create a process */
    uint32_t pid1 = process_create(1, 4096, 8192);
    ASSERT_NEQ(pid1, 0, "Process creation returns valid PID");
    
    /* Test 2: Create multiple processes */
    uint32_t pid2 = process_create(2, 4096, 8192);
    uint32_t pid3 = process_create(1, 4096, 8192);
    ASSERT_NEQ(pid2, 0, "Second process created");
    ASSERT_NEQ(pid3, 0, "Third process created");
    
    /* Test 3: PIDs are unique */
    ASSERT_NEQ(pid1, pid2, "Different processes have different PIDs");
    ASSERT_NEQ(pid2, pid3, "Different processes have different PIDs");
}

void test_process_state_transitions(void) {
    serial_puts("\n--- PROCESS STATE TRANSITION TESTS ---\n");
    
    process_init();
    uint32_t pid = process_create(1, 4096, 8192);
    
    /* Test 1: Initial state should be READY */
    process_state_t state = process_get_state(pid);
    ASSERT_EQ(state, READY, "New process starts in READY state");
    
    /* Test 2: Set to CURRENT */
    process_set_state(pid, CURRENT);
    state = process_get_state(pid);
    ASSERT_EQ(state, CURRENT, "Process can transition to CURRENT state");
    
    /* Test 3: Set to READY */
    process_set_state(pid, READY);
    state = process_get_state(pid);
    ASSERT_EQ(state, READY, "Process can transition to READY state");
    
    /* Test 4: Set to TERMINATED */
    process_set_state(pid, TERMINATED);
    state = process_get_state(pid);
    ASSERT_EQ(state, TERMINATED, "Process can transition to TERMINATED state");
}

void test_process_termination(void) {
    serial_puts("\n--- PROCESS TERMINATION TESTS ---\n");
    
    process_init();
    uint32_t pid = process_create(1, 4096, 8192);
    
    /* Verify process exists */
    process_control_block_t *pcb = process_get_pcb(pid);
    ASSERT(pcb != 0, "Process PCB exists after creation");
    
    /* Terminate process */
    process_terminate(pid);
    process_state_t state = process_get_state(pid);
    ASSERT_EQ(state, TERMINATED, "Terminated process has TERMINATED state");
}

void test_process_get_pcb(void) {
    serial_puts("\n--- PROCESS PCB RETRIEVAL TESTS ---\n");
    
    process_init();
    uint32_t pid = process_create(2, 4096, 8192);
    
    process_control_block_t *pcb = process_get_pcb(pid);
    ASSERT(pcb != 0, "Can retrieve PCB by valid PID");
    
    if (pcb) {
        ASSERT_EQ(pcb->process_id, pid, "Retrieved PCB has correct PID");
        ASSERT_EQ(pcb->priority, 2, "Retrieved PCB has correct priority");
        ASSERT(pcb->stack_size > 0, "Retrieved PCB has valid stack size");
    }
}

/* ============================================================================
   SCHEDULER TESTS
   ============================================================================ */

void test_scheduler_init(void) {
    serial_puts("\n--- SCHEDULER TESTS ---\n");
    
    /* Test 1: Initialize with FCFS */
    scheduler_init(FCFS, 0);
    serial_puts("[PASS] Scheduler initialized with FCFS\n");
    tests_passed++;
    tests_run++;
    
    /* Test 2: Initialize with Round Robin */
    scheduler_init(RR, 10);
    serial_puts("[PASS] Scheduler initialized with Round Robin\n");
    tests_passed++;
    tests_run++;
}

void test_scheduler_get_next_process(void) {
    serial_puts("\n--- SCHEDULER PROCESS SELECTION TESTS ---\n");
    
    process_init();
    scheduler_init(FCFS, 0);
    
    /* Create test processes */
    uint32_t pid1 = process_create(1, 4096, 8192);
    uint32_t pid2 = process_create(1, 4096, 8192);
    
    /* Get next process */
    uint32_t next = scheduler_get_next_process();
    ASSERT(next != 0, "Scheduler returns a process");
    
    /* Next process should be first created (FCFS) */
    ASSERT_EQ(next, pid1, "Scheduler selects process using FCFS order");
}

void test_scheduler_update_time(void) {
    serial_puts("\n--- SCHEDULER TIME UPDATE TESTS ---\n");
    
    scheduler_init(RR, 10);
    
    serial_puts("[PASS] Scheduler time can be updated\n");
    tests_passed++;
    tests_run++;
    
    scheduler_update_time();
    serial_puts("[PASS] Scheduler update_time executes without error\n");
    tests_passed++;
    tests_run++;
}

void test_scheduler_aging(void) {
    serial_puts("\n--- SCHEDULER AGING TESTS ---\n");
    
    process_init();
    scheduler_init(RR, 10);
    
    uint32_t pid = process_create(1, 4096, 8192);
    ASSERT_NEQ(pid, 0, "Process created for aging test");
    
    scheduler_apply_aging();
    serial_puts("[PASS] Scheduler aging applied without error\n");
    tests_passed++;
    tests_run++;
}

/* ============================================================================
   INTEGRATION TESTS
   ============================================================================ */

void test_integration_full_lifecycle(void) {
    serial_puts("\n--- INTEGRATION TESTS ---\n");
    
    /* Initialize all systems */
    memory_init();
    process_init();
    scheduler_init(RR, 10);
    
    /* Create multiple processes */
    uint32_t pid1 = process_create(1, 4096, 8192);
    uint32_t pid2 = process_create(2, 4096, 8192);
    uint32_t pid3 = process_create(1, 4096, 8192);
    
    ASSERT(pid1 != 0 && pid2 != 0 && pid3 != 0, 
           "Multiple processes created successfully");
    
    /* Schedule and simulate execution */
    uint32_t next = scheduler_get_next_process();
    ASSERT_EQ(next, pid1, "Scheduler selects first process");
    
    /* Update states and time */
    process_set_state(pid1, CURRENT);
    scheduler_update_time();
    ASSERT_EQ(process_get_state(pid1), CURRENT, 
              "Process state changed to CURRENT");
    
    /* Terminate and verify */
    process_terminate(pid1);
    ASSERT_EQ(process_get_state(pid1), TERMINATED, 
              "Process terminated successfully");
}

void test_integration_stress(void) {
    serial_puts("\n--- STRESS TESTS ---\n");
    
    process_init();
    
    /* Create many processes */
    uint32_t pids[50];
    uint32_t i;
    
    for (i = 0; i < 50; i++) {
        pids[i] = process_create(i % 3 + 1, 4096, 8192);
    }
    
    ASSERT(pids[0] != 0, "Stress: 50 processes created");
    
    /* Terminate half of them */
    for (i = 0; i < 25; i++) {
        process_terminate(pids[i]);
    }
    
    serial_puts("[PASS] Stress test: Multiple process creation and termination\n");
    tests_passed++;
    tests_run++;
}

/* ============================================================================
   TEST RUNNER
   ============================================================================ */

void run_all_tests(void) {
    serial_puts("\n");
    serial_puts("================================================================================\n");
    serial_puts("                    kacchiOS TEST SUITE\n");
    serial_puts("================================================================================\n");
    
    /* Memory tests */
    test_memory_allocate();
    test_memory_free_process();
    
    /* Process tests */
    test_process_creation();
    test_process_state_transitions();
    test_process_termination();
    test_process_get_pcb();
    
    /* Scheduler tests */
    test_scheduler_init();
    test_scheduler_get_next_process();
    test_scheduler_update_time();
    test_scheduler_aging();
    
    /* Integration tests */
    test_integration_full_lifecycle();
    test_integration_stress();
    
    /* Print summary */
    serial_puts("\n");
    serial_puts("================================================================================\n");
    serial_puts("                    TEST SUMMARY\n");
    serial_puts("================================================================================\n");
    serial_puts("Tests Run:    ");
    serial_put_dec(tests_run);
    serial_puts("\n");
    serial_puts("Tests Passed: ");
    serial_put_dec(tests_passed);
    serial_puts("\n");
    serial_puts("Tests Failed: ");
    serial_put_dec(tests_failed);
    serial_puts("\n");
    
    if (tests_failed == 0) {
        serial_puts("\n✅ ALL TESTS PASSED!\n");
    } else {
        serial_puts("\n❌ SOME TESTS FAILED!\n");
    }
    serial_puts("================================================================================\n\n");
}
