#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/UDPEchoClient.out -I ../src
./../../../bin/UDPEchoClient.out $*