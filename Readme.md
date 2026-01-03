# kacchiOS - A Baremetal OS from Scratch

This is my implementation of a minimal operating system kernel in x86 assembly and C. Started as a lab assignment to understand how processes, memory, and scheduling actually work at the hardware level.

## What I Built

I implemented three core OS components from scratch:

**1. Memory Manager** - The tricky part was balancing simplicity with efficiency. I went with a hybrid approach: bump allocation for speed, but also added free-block reuse to avoid exhausting the heap too quickly. Added tail compaction so when the last few blocks are freed, we actually reclaim that space instead of fragmenting forever. Also implemented double-free detection because that cost me hours debugging once.

**2. Process Manager** - Straightforward process table with PCBs, but the challenge was getting the CPU context right. Each process needs its own stack, heap, and CPU state for context switching. I track process creation time and wait time for the scheduler's aging mechanism.

**3. Scheduler** - Implemented both FCFS and Round Robin. The interesting part was aging: processes waiting too long get priority bumped automatically. This prevents starvation, which is a real problem in simple round-robin schedulers.

All three components integrate: when a process terminates, memory gets freed; the scheduler picks the next process to run; context switch happens. The full test suite validates this interaction.

## Getting It Running

I'm assuming you have QEMU and a 32-bit GCC toolchain:

```bash
# Linux (Ubuntu/Debian)
sudo apt-get install build-essential qemu-system-x86 gcc-multilib binutils

# Build and test
make clean
make
./run_tests.sh
```

To boot the actual OS:
```bash
qemu-system-i386 -kernel kernel.elf -serial mon:stdio
```

Type something - it echoes back through the null process. Nothing fancy, but it works.

## How It's Organized

```
boot.S              - x86 bootloader, sets up protected mode
kernel.c            - Main kernel loop, initializes subsystems
serial.c/h          - COM1 driver for debug output
string.c/h          - Basic libc functions (strcpy, memcpy, etc)

memory.c/h          - Memory allocator with reuse + compaction
process.c/h         - Process table and lifecycle management  
scheduler.c/h       - FCFS/RR scheduler with aging
test_suite.c        - 40 test cases covering all three components

link.ld             - Linker script (memory layout)
Makefile            - Build rules
```

The test suite is crucial - it validates that memory, processes, and scheduler actually work together. I run it with QEMU and check for memory fragmentation, process state correctness, and scheduler fairness.

## The Memory Manager

The hardest part: balancing simplicity vs. avoiding fragmentation.

**What I did:**
- Simple bump allocator for speed (just increment a pointer)
- Free-block reuse: when a process dies, scan for a free block large enough and reuse it
- Tail compaction: if the last block(s) are free, we reclaim that heap space instead of leaking it
- Double-free detection: tried to free the same address twice? We catch it now

**Why this approach:**
- Bump allocation is O(1) - can't beat that
- Free-block reuse prevents pathological fragmentation (alloc → free → alloc → free... forever)
- Tail compaction reclaims peak memory without complex coalescing logic
- Works well for small kernel heaps where fragmentation isn't the primary concern

**The code:**
- `memory_allocate()` checks free blocks first, then bumps the heap
- `memory_compact_tail()` runs after every free, recalculates heap_pointer
- `memory_free_process()` marks all blocks for a process as FREE, then compacts

Test case: allocate 1KB, free it, allocate 512B → reuses the same address. No wasted space.

## The Process Manager

Fairly straightforward once you understand PCBs.

**What's in a Process Control Block (PCB):**
- Process ID, priority, state (READY/CURRENT/TERMINATED)
- Stack base + size (both allocated from heap)
- Heap base + size
- CPU context (registers: esp, ebp, eip, eflags, etc.)
- Metadata: creation_time, wait_time (for aging)

**Process lifecycle:**
1. Create: allocate stack + heap from memory manager, init PCB, set state to READY
2. Schedule: pick next process, context switch
3. Terminate: set state to TERMINATED, free all memory, clean up PCB

**Why separate stack and heap:**
- Stack grows down (push/pop for function calls)
- Heap grows up (malloc-style allocation)
- Both allocated from the same pool but managed differently
- Process can't access kernel memory (0x00-0x10000 is kernel only)

**The tricky part:** CPU context. Each process needs its own register snapshot so we can swap them in/out. I store esp (stack pointer) pointing to the top of each process's stack. When scheduler does a context switch, it swaps these registers.

## The Scheduler

Implemented two algorithms: FCFS and Round Robin with aging.

**FCFS (First Come First Served):**
- Simple: just pick the first READY process by PID (creation order)
- Low overhead but unfair: if one process hogs CPU, others starve
- Used mostly for testing

**Round Robin with Aging:**
- Each process gets a time quantum (e.g., 10ms)
- After quantum expires, preempt and pick next READY process
- Fairer, but aging is what makes it smart

**Aging mechanism:**
- Every process waiting longer than 1000ms gets priority bumped
- Wait time is incremented in `scheduler_update_time()` (called from timer)
- This prevents starvation: even low-priority processes eventually run
- Once a process gets bumped, its wait_time resets

**Why aging matters:**
Without it: low-priority process never runs (starves)  
With it: low-priority process waits a bit, priority increases, eventually runs

**The implementation:**
- `scheduler_get_next_process()` scans READY processes, returns best candidate
- `scheduler_context_switch()` sets old process to READY, new process to CURRENT
- `scheduler_update_time()` increments wait times, triggers aging
- Time quantum and algorithm are configurable at init time

## Testing & Validation

I wrote 40 test cases covering:

**Memory tests:**
- Basic allocation returns valid address
- Multiple allocations don't collide
- Zero-size allocation rejected
- Free-block reuse works (allocate → free → allocate)
- Process cleanup frees all blocks

**Process tests:**
- Process creation succeeds, returns unique PID
- State transitions (READY → CURRENT → TERMINATED)
- Termination properly frees memory
- PCB retrieval works

**Scheduler tests:**
- FCFS picks processes in creation order
- Round Robin respects time quantum
- Aging actually bumps priority

**Integration tests:**
- Full lifecycle: create processes, schedule, terminate
- 50-process stress test: create, terminate 25 of them, verify cleanup
- Memory is reclaimed (no leaks after bulk termination)

**Result:** 40/40 tests pass. The system handles concurrent creation/termination without corruption or memory leaks.

## Build & Test

```bash
# Build everything
make clean
make

# Run the test suite (QEMU)
./run_tests.sh

# Or manually test the OS
qemu-system-i386 -kernel kernel.elf -serial mon:stdio
```

Makefile has standard targets: `all`, `clean`. The `run_tests.sh` script compiles the test kernel and runs it under QEMU, capturing output to verify all 40 tests pass.

## Why This Matters

Understanding memory allocation, process management, and scheduling isn't just academic. Real systems (Linux, Windows, etc.) do exactly this:
- Allocate memory for processes and prevent fragmentation
- Manage process lifecycle (fork/exec/exit)
- Schedule fairly and prevent starvation

This is a simplified version, but the core concepts are the same. Building it from scratch makes those concepts concrete.
