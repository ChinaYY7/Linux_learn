#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/TCPEchoServer.out -I ../src -lpthread
clear
./../../../bin/TCPEchoServer.out $*
