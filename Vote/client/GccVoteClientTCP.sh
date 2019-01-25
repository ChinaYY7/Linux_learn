#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/VoteClientTCP.out -I ../src
./../../../bin/VoteClientTCP.out $*