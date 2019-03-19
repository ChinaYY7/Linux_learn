#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/Ftp_Client.out -I ../src
#clear
./../../../bin/Ftp_Client.out $*
