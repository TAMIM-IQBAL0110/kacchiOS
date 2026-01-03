#!/bin/bash

# Test runner script for kacchiOS
# This script builds and runs the test suite

set -e

PROJECT_DIR="/home/tamim/os-lab/kacchiOS"

echo "=========================================="
echo "    kacchiOS Test Runner"
echo "=========================================="
echo ""

# Build the project
echo "[1] Building kacchiOS..."
cd "$PROJECT_DIR"
make clean > /dev/null 2>&1
make > /dev/null 2>&1
echo "✓ Production kernel built successfully"

# Build test kernel
echo ""
echo "[2] Building test kernel..."
gcc -m32 -ffreestanding -O2 -Wall -Wextra -nostdinc -fno-builtin -fno-stack-protector -I. -c test_kernel.c -o test_kernel.o > /dev/null 2>&1
gcc -m32 -ffreestanding -O2 -Wall -Wextra -nostdinc -fno-builtin -fno-stack-protector -I. -c test_suite.c -o test_suite.o > /dev/null 2>&1
ld -m elf_i386 -T link.ld -o test_kernel.elf boot.o test_kernel.o serial.o string.o memory.o process.o scheduler.o test_suite.o > /dev/null 2>&1
echo "✓ Test kernel built successfully"

# Run tests
echo ""
echo "[3] Running test suite..."
echo "=========================================="
timeout 5 qemu-system-i386 -kernel test_kernel.elf -m 64M -serial stdio -display none 2>&1 || true

# Extract test results
echo ""
echo "=========================================="
echo "Build Artifacts:"
echo "=========================================="
ls -lh kernel.elf test_kernel.elf 2>/dev/null | awk '{print $9, "(" $5 ")"}'

echo ""
echo "=========================================="
echo "Compilation Status: ✓ SUCCESS"
echo "Test Execution: ✓ COMPLETED"
echo "=========================================="
