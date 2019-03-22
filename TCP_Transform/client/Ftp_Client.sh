#!/bin/bash
#gcc -g *.c ../src/*.c -o ../../../bin/Ftp_Client.out -I ../src
make
#clear
././bin/Ftp_Client.bin $*
