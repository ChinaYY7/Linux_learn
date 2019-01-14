#!/bin/bash
gcc -g MemoryPooltest.c MemoryPool.c -o ../../bin/MemoryPooltest.out
cd ../../bin
./MemoryPooltest.out
