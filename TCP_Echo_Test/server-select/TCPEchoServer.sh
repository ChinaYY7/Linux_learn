#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/TCPEchoServer.out -I ../src
clear
./../../../bin/TCPEchoServer.out $* #4000 4001 4002 4003 4004
