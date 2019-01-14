#!/bin/bash
gcc -g *.c -o ../../../bin/TCPEchoClient.out
#gcc -g TCPEchoClient.c Deal_Error.c -o ../../bin/TCPEchoClient.out
#cd ../../bin
echo $*
./../../../bin/TCPEchoClient.out 127.0.0.1 "Echo This" 5000
