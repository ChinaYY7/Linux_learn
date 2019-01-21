#!/bin/bash
gcc -g *.c -o ../../../bin/TCPEchoServer.out
#gcc -g TCPEchoClient.c Deal_Error.c -o ../../bin/TCPEchoClient.out
#cd ../../bin
./../../../bin/TCPEchoServer.out $*
