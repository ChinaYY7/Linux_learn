#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/UDPEchoServer.out -I ../src
clear
./../../../bin/UDPEchoServer.out $*
