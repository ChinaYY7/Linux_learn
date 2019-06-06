#!/bin/bash
#gcc -g *.c ../src/*.c -o ../../../bin/Ftp_Client.out -I ../src
make
#clear
echo "Starting Progress ..."
././bin/S_Server.bin $*
