vpath %.c ../src
vpath %.h ../src 
vpath %.o ./object

object_path=./object
bin_path=./bin
src_path=../src
target=$(bin_path)/Ftp_Client.bin
objects=Ftp_Client.o Ftp_ClientUtility.o AddressUtility.o Deal_Error.o Encode.o Framer.o Trans_Protocol.o
CFLAGS=-I ../src

$(target) : $(objects)
	cd object && cc -g $(objects) -o ../$(target) $(CFLAGS) 

Ftp_Client.o : Ftp_Client.c *.h ../src/*.h
	cc -c -g Ftp_Client.c -o $(object_path)/Ftp_Client.o $(CFLAGS)

Ftp_ClientUtility.o : Ftp_ClientUtility.c *.h ../src/*.h
	cc -c -g Ftp_ClientUtility.c -o $(object_path)/Ftp_ClientUtility.o $(CFLAGS) 

AddressUtility.o : AddressUtility.c AddressUtility.h apue.h
	cc -c -g $(src_path)/AddressUtility.c -o $(object_path)/AddressUtility.o $(CFLAGS) 

Deal_Error.o : Deal_Error.c Deal_Error.h apue.h
	cc -c -g $(src_path)/Deal_Error.c -o $(object_path)/Deal_Error.o $(CFLAGS) 

Trans_Protocol.o : Trans_Protocol.c  Trans_Protocol.h Encode.h Framer.h apue.h Deal_Error.h
	cc -c -g $(src_path)/Trans_Protocol.c -o $(object_path)/Trans_Protocol.o $(CFLAGS)

Encode.o : Encode.c Encode.h Trans_Protocol.h apue.h
	cc -c -g $(src_path)/Encode.c -o $(object_path)/Encode.o $(CFLAGS) 

Framer.o : Framer.c Framer.h Deal_Error.h apue.h
	cc -c -g $(src_path)/Framer.c -o $(object_path)/Framer.o $(CFLAGS) 

.PHONY: clean
clean :
	-rm $(target) $(object_path)/*.o