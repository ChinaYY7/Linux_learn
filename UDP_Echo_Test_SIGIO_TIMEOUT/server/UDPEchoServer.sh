#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/UDPEchoServer.out -I ../src
./../../../bin/UDPEchoServer.out $*
