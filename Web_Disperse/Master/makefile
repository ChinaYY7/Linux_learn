vpath %.c ../src
vpath %.h ../src 
vpath %.o ./object

object_path=./object
bin_path=./bin
src_path=../src
target=$(bin_path)/M_Server.bin
objects=M_Server.o M_ServerUtility.o TCP_Socket.o Deal_Error.o Trans_Protocol.o
CFLAGS=-I ../src

$(target) : $(objects)
	cd object && cc -g $(objects) -o ../$(target) $(CFLAGS) 

M_Server.o : M_Server.c *.h ../src/*.h
	cc -c M_Server.c -o $(object_path)/M_Server.o $(CFLAGS)

M_ServerUtility.o : M_ServerUtility.c *.h ../src/*.h
	cc -c M_ServerUtility.c -o $(object_path)/M_ServerUtility.o $(CFLAGS) 

TCP_Socket.o : TCP_Socket.c TCP_Socket.h Deal_Error.h apue.h
	cc -c $(src_path)/TCP_Socket.c -o $(object_path)/TCP_Socket.o $(CFLAGS) 

Deal_Error.o : Deal_Error.c Deal_Error.h apue.h
	cc -c $(src_path)/Deal_Error.c -o $(object_path)/Deal_Error.o $(CFLAGS) 

Trans_Protocol.o : Trans_Protocol.c  Trans_Protocol.h TCP_Socket.h apue.h Deal_Error.h
	cc -c -g $(src_path)/Trans_Protocol.c -o $(object_path)/Trans_Protocol.o $(CFLAGS)

.PHONY: clean
clean :
	-rm $(target) $(object_path)/*.o