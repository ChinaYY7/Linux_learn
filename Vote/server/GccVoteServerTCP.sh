#!/bin/bash
gcc -g *.c ../src/*.c -o ../../../bin/VoteServerTCP.out -I ../src
#gcc -g TCPEchoClient.c Deal_Error.c -o ../../bin/TCPEchoClient.out
#cd ../../bin
./../../../bin/VoteServerTCP.out $*
