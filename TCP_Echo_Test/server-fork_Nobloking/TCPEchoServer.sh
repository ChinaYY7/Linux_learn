#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/TCPEchoServer.out -I ../src
#clear
echo "Starting Progress ..."
./../../../bin/TCPEchoServer.out $*
