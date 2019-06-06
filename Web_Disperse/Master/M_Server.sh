#!/bin/bash
make
#gcc -g *.c ../src/*.c -o ./bin/Ftp_Server.bin -I ../src
#clear
echo "Starting Progress ..."
././bin/M_Server.bin $*
