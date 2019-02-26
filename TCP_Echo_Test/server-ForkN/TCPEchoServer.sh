#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/TCPEchoServer.out -I ../src
clear
./../../../bin/TCPEchoServer.out $*
