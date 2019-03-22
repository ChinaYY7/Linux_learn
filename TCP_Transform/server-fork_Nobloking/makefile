vpath %.c ../src
vpath %.h ../src 
vpath %.o ./object

object_path=./object
bin_path=./bin
src_path=../src
target=$(bin_path)/Ftp_Server.bin
objects=Ftp_Server.o Ftp_ServerUtility.o AddressUtility.o Deal_Error.o Encode.o Framer.o
CFLAGS=-I ../src

$(target) : $(objects)
	cd object && cc -g $(objects) -o ../$(target) $(CFLAGS) 

Ftp_Server.o : Ftp_Server.c *.h ../src/*.h
	cc -c Ftp_Server.c -o $(object_path)/Ftp_Server.o $(CFLAGS)

Ftp_ServerUtility.o : Ftp_ServerUtility.c Ftp_ServerUtility.h AddressUtility.h Deal_Error.h apue.h
	cc -c Ftp_ServerUtility.c -o $(object_path)/Ftp_ServerUtility.o $(CFLAGS) 

AddressUtility.o : AddressUtility.c AddressUtility.h apue.h
	cc -c $(src_path)/AddressUtility.c -o $(object_path)/AddressUtility.o $(CFLAGS) 

Deal_Error.o : Deal_Error.c Deal_Error.h apue.h
	cc -c $(src_path)/Deal_Error.c -o $(object_path)/Deal_Error.o $(CFLAGS) 

Encode.o : Encode.c Encode.h Trans_Protocol.h
	cc -c $(src_path)/Encode.c -o $(object_path)/Encode.o $(CFLAGS) 

Framer.o : Framer.c Framer.h Deal_Error.h apue.h
	cc -c $(src_path)/Framer.c -o $(object_path)/Framer.o $(CFLAGS) 

.PHONY: clean
clean :
	-rm $(target) $(object_path)/*.o