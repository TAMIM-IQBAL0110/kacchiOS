#include "memory.h"
#include "serial.h"
#include "string.h"

static memory_allocator_t allocator;
static uint32_t heap_pointer;

static void memory_compact_tail(void) {
    while (allocator.block_count > 0) {
        memory_block_t *last = &allocator.blocks[allocator.block_count - 1];
        if (last->state != FREE) {
            break;
        }
        allocator.block_count--;
    }
    if (allocator.block_count == 0) {
        heap_pointer = allocator.heap_start;
    } 
    else {
        memory_block_t *last = &allocator.blocks[allocator.block_count - 1];
        heap_pointer = last->address + last->size;
    }
}
static memory_block_t* memory_find_block(uint32_t address) {
    uint32_t i;
    for (i = 0; i < allocator.block_count; i++) {
        if (allocator.blocks[i].address == address) {
            return &allocator.blocks[i];
        }
    }
    return NULL;
}
void memory_init(void) {
    allocator.heap_start = PROCESS_HEAP_START;
    allocator.heap_end = PROCESS_HEAP_START + PROCESS_HEAP_SIZE;
    allocator.block_count = 0;
    heap_pointer = allocator.heap_start;
    
    serial_puts("[MEMORY] Memory allocator initialized\n");
}

/**
 * Allocate memory for a process
 * @param size: Size of memory to allocate
 * @param process_id: ID of the process requesting memory
 * @return: Address of allocated memory, or 0 on failure
 */
uint32_t memory_allocate(uint32_t size, uint32_t process_id) {
    uint32_t i;
    if (size == 0) {
        serial_puts("[MEMORY] ERROR: Zero-size allocation requested\n");
        return 0;
    }
    for (i = 0; i < allocator.block_count; i++) {
        memory_block_t *block = &allocator.blocks[i];

        if (block->state == FREE && block->size >= size) {
            block->state = ALLOCATED;
            block->process_id = process_id;
            return block->address;
        }
    }
    if (allocator.block_count >= MAX_MEMORY_BLOCKS) {
        serial_puts("[MEMORY] ERROR: Maximum memory blocks reached\n");
        return 0;
    }
    if (heap_pointer + size > allocator.heap_end) {
        serial_puts("[MEMORY] ERROR: Heap exhausted\n");
        return 0;
    }
    
    uint32_t allocated_addr = heap_pointer;
    memory_block_t *block = &allocator.blocks[allocator.block_count];
    block->address = allocated_addr;
    block->size = size;
    block->state = ALLOCATED;
    block->process_id = process_id;
    allocator.block_count++;
    heap_pointer += size;
    return allocated_addr;
}

/**
 * Free memory at a specific address
 * @param address: Address of memory to free
 */
void memory_free(uint32_t address) {
    memory_block_t *block = memory_find_block(address);

    if (block == NULL) {
        serial_puts("[MEMORY] WARNING: Attempted to free unallocated address\n");
        return;
    }
    if (block->state == FREE) {
        serial_puts("[MEMORY] WARNING: Attempted double free detected\n");
        return;
    }
    block->state = FREE;
    memory_compact_tail();
}

/**
 * Free all memory allocated to a process
 * @param process_id: ID of process whose memory to free
 */
void memory_free_process(uint32_t process_id) {
    uint32_t i;
    uint32_t freed_count = 0;
    uint32_t freed_bytes = 0;
    for (i = 0; i < allocator.block_count; i++) {
        if (allocator.blocks[i].process_id == process_id && 
            allocator.blocks[i].state == ALLOCATED) {
            allocator.blocks[i].state = FREE;
            freed_count++;
            freed_bytes += allocator.blocks[i].size;
        }
    }
    if (freed_count == 0) {
        serial_puts("[MEMORY] WARNING: No allocated blocks found for process\n");
        return;
    }
    memory_compact_tail();
    serial_puts("[MEMORY] Freed ");
    serial_put_dec(freed_bytes);
    serial_puts(" bytes across ");
    serial_put_dec(freed_count);
    serial_puts(" blocks for process ");
    serial_put_dec(process_id);
    serial_puts("\n");
}

//Print memory allocator status
void memory_print_status(void) {
    uint32_t i;
    uint32_t total_allocated = 0;
    uint32_t total_free = 0;
    uint32_t unallocated_tail = allocator.heap_end - heap_pointer;
    serial_puts("\n=== Memory Status ===\n");
    serial_puts("Block Address | Size      | State    | Process ID\n");
    serial_puts("----------------------------------------------\n");
    for (i = 0; i < allocator.block_count; i++) {
        memory_block_t *block = &allocator.blocks[i];
        if (block->state == ALLOCATED) {
            total_allocated += block->size;
        } 
        else {
            total_free += block->size;
        }
        serial_puts("0x");
        serial_put_hex(block->address);
        serial_puts(" | ");
        serial_put_dec(block->size);
        serial_puts(" bytes | ");
        serial_puts(block->state == ALLOCATED ? "ALLOCATED" : "FREE     ");
        serial_puts(" | ");
        serial_put_dec(block->process_id);
        serial_puts("\n");
    }
    serial_puts("----------------------------------------------\n");
    serial_puts("Total Allocated: ");
    serial_put_dec(total_allocated);
    serial_puts(" bytes\n");
    serial_puts("Total Free: ");
    serial_put_dec(total_free + unallocated_tail);
    serial_puts(" bytes\n");
    serial_puts("Unallocated Heap: ");
    serial_put_dec(unallocated_tail);
    serial_puts(" bytes\n");
    serial_puts("Heap Pointer: 0x");
    serial_put_hex(heap_pointer);
    serial_puts("\n\n");
}
