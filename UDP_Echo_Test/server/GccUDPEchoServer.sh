#!/bin/bash
gcc -g *.c -o ../../../bin/UDPEchoServer.out
./../../../bin/UDPEchoServer.out $*
