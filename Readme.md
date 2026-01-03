# 🍚 kacchiOS

A minimal, educational baremetal operating system designed for teaching OS fundamentals.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86-lightgrey.svg)]()

## 📖 Overview

kacchiOS is a simple, bare-metal operating system built from scratch for educational purposes. It provides a clean foundation for students to learn operating system concepts by implementing core components themselves.

### Current Features

- ✅ **Multiboot-compliant bootloader** - Boots via GRUB/QEMU
- ✅ **Serial I/O driver** (COM1) - Communication via serial port
- ✅ **Null process** - Single process that reads and echoes input
- ✅ **Basic string utilities** - Essential string operations
- ✅ **Memory Manager** - Stack and heap allocation/deallocation
- ✅ **Process Manager** - Process creation, termination, state transitions
- ✅ **Scheduler** - FCFS and Round Robin scheduling with aging
- ✅ **Context Switching** - Process context management
- ✅ **Clean, documented code** - Easy to understand and extend

### Implemented Components (Lab Assignment)

✅ **Memory Manager** - Allocates and deallocates memory for processes
✅ **Process Manager** - Manages process lifecycle (creation, termination, state)
✅ **Scheduler** - Schedules processes with configurable algorithms

## 🚀 Quick Start

### Prerequisites

```bash
# On Ubuntu/Debian
sudo apt-get install build-essential qemu-system-x86 gcc-multilib

# On Arch Linux
sudo pacman -S base-devel qemu gcc-multilib

# On macOS
brew install qemu i686-elf-gcc
```

### Build and Run

```bash
# Clone the repository
git clone https://github.com/yourusername/kacchiOS.git
cd kacchiOS

# Build the OS
make clean
make

# Run in QEMU
make run
```

You should see:
```
========================================
    kacchiOS - Minimal Baremetal OS
========================================
Hello from kacchiOS!
Running null process...

kacchiOS> 
```

Type something and press Enter - it will echo back!

## 📁 Project Structure

```
kacchiOS/
├── src/
│   ├── boot.S           # Bootloader entry point (Assembly)
│   ├── kernel.c         # Main kernel (null process)
│   ├── serial.c         # Serial port driver (COM1)
│   ├── serial.h         # Serial driver interface
│   ├── string.c         # String utility functions
│   ├── string.h         # String utility interface
│   ├── types.h          # Basic type definitions
│   ├── io.h             # I/O port operations
│   ├── memory.c         # Memory manager implementation
│   ├── memory.h         # Memory manager interface
│   ├── process.c        # Process manager implementation
│   ├── process.h        # Process manager interface
│   ├── scheduler.c      # Scheduler implementation
│   ├── scheduler.h      # Scheduler interface
│   ├── link.ld          # Linker script
│   └── Makefile         # Build system
└── docs/
    ├── IMPLEMENTATION.md # Implementation details
    └── CHECKLIST.md     # Checklist status
```

## 🔑 Key Components

### Memory Manager (memory.c/h)

Provides heap-based memory allocation for processes.

**Features:**
- Stack allocation during process creation
- Dynamic heap allocation (`memory_allocate()`)
- Memory block tracking and deallocation
- Memory status reporting (`memory_print_status()`)

**Example Usage:**
```c
memory_init();
uint32_t addr = memory_allocate(1024, process_id);
memory_free(addr);
memory_free_process(process_id);  // Free all process memory
```

### Process Manager (process.c/h)

Manages the complete lifecycle of processes.

**Features:**
- Process table (up to 256 processes)
- Process creation with memory allocation
- State management (TERMINATED, READY, CURRENT)
- Process termination and cleanup
- CPU context switching support

**Process States:**
```
TERMINATED ← CURRENT ↔ READY
```

**Example Usage:**
```c
process_init();
uint32_t pid = process_create(10, 0x1000, 0x2000);  // priority, stack, heap
process_set_state(pid, READY);
process_terminate(pid);
```

### Scheduler (scheduler.c/h)

Implements process scheduling algorithms.

**Algorithms:**
- **FCFS** (First Come First Served) - Simple priority-based scheduling
- **Round Robin with Aging** - Time-sliced scheduling with starvation prevention

**Features:**
- Configurable time quantum for Round Robin
- Aging mechanism to boost long-waiting processes
- Context switching between processes
- Time tracking and scheduling decisions

**Example Usage:**
```c
scheduler_init(RR, 10);  // Round Robin with 10ms quantum
scheduler_get_next_process();
scheduler_context_switch(from_pid, to_pid);
scheduler_update_time();  // Called from timer interrupt
```

## 🛠️ Build System

### Makefile Targets

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build kernel.elf |
| `make run` | Run in QEMU (serial output only) |
| `make run-vga` | Run in QEMU (with VGA window) |
| `make debug` | Run in debug mode (GDB ready) |
| `make clean` | Remove build artifacts |

## 📚 Learning Resources

### Recommended Reading

- [XINU OS](https://xinu.cs.purdue.edu/) - Educational OS similar to kacchiOS
- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development guide
- [The Little OS Book](https://littleosbook.github.io/) - Practical OS development
- [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/) - OS concepts textbook

### Related Topics

- x86 Assembly Language
- Memory Management
- Process Scheduling
- System Calls
- Interrupt Handling

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### Guidelines

1. Keep code simple and educational
2. Add comments explaining complex concepts
3. Follow existing code style
4. Test changes in QEMU before submitting

## 📄 License

This project is licensed under the MIT License.

## 👨‍🏫 About

kacchiOS was created as an educational tool for teaching operating system concepts. It provides a minimal, working foundation that students can extend to learn core OS principles through hands-on implementation.

## 🙏 Acknowledgments

- Inspired by XINU OS
- Built with guidance from OSDev community
- Thanks to all students who have contributed
