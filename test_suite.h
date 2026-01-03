/* test_suite.h - Test suite header for kacchiOS */
#ifndef TEST_SUITE_H
#define TEST_SUITE_H

/* Run all tests */
void run_all_tests(void);

/* Memory manager tests */
void test_memory_allocate(void);
void test_memory_free_process(void);

/* Process manager tests */
void test_process_creation(void);
void test_process_state_transitions(void);
void test_process_termination(void);
void test_process_get_pcb(void);

/* Scheduler tests */
void test_scheduler_init(void);
void test_scheduler_get_next_process(void);
void test_scheduler_update_time(void);
void test_scheduler_aging(void);

/* Integration tests */
void test_integration_full_lifecycle(void);
void test_integration_stress(void);

#endif
