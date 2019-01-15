#!/bin/bash
gcc -g *.c -o ../../../bin/TCPEchoClient.out
./../../../bin/TCPEchoClient.out $*
