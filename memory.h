/* memory.h - Memory manager for kacchiOS */
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Memory configuration */
#define KERNEL_HEAP_START   0x10000
#define KERNEL_HEAP_SIZE    0x100000      /* 1 MB */
#define PROCESS_HEAP_START  0x110000
#define PROCESS_HEAP_SIZE   0x400000      /* 4 MB */
#define MAX_MEMORY_BLOCKS   256

/* Memory block states */
typedef enum {
    FREE,
    ALLOCATED
} block_state_t;

/* Memory block metadata */
typedef struct {
    uint32_t address;
    uint32_t size;
    block_state_t state;
    uint32_t process_id;    /* ID of process that owns this block */
} memory_block_t;

/* Memory allocator structure */
typedef struct {
    memory_block_t blocks[MAX_MEMORY_BLOCKS];
    uint32_t block_count;
    uint32_t heap_start;
    uint32_t heap_end;
} memory_allocator_t;

/* Function declarations */
void memory_init(void);
uint32_t memory_allocate(uint32_t size, uint32_t process_id);
void memory_free(uint32_t address);
void memory_free_process(uint32_t process_id);
void memory_print_status(void);

#endif
