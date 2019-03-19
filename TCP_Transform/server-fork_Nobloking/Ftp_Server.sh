#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/Ftp_Server.out -I ../src
#clear
echo "Starting Progress ..."
./../../../bin/Ftp_Server.out $*
