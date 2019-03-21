target=./bin/Ftp_Server.bin
objects=./object/Ftp_Server.o ./object/Ftp_ServerUtility.o ./object/AddressUtility.o ./object/Deal_Error.o ./object/Encode.o ./object/Framer.o

$(target) : $(objects) ../src/apue.h
	cc -g $(objects) -o $(target) -I ../src

./object/Ftp_Server.o : Ftp_Server.c ../src/*.h
	cc -c Ftp_Server.c -o ./object/Ftp_Server.o -I ../src

./object/Ftp_ServerUtility.o : Ftp_ServerUtility.c ../src/*.h
	cc -c Ftp_ServerUtility.c -o ./object/Ftp_ServerUtility.o -I ../src

./object/AddressUtility.o : ../src/AddressUtility.c ../src/*.h
	cc -c ../src/AddressUtility.c -o ./object/AddressUtility.o -I ../src

./object/Deal_Error.o : ../src/Deal_Error.c ../src/*.h
	cc -c ../src/Deal_Error.c -o ./object/Deal_Error.o -I ../src

./object/Encode.o : ../src/Encode.c ../src/*.h
	cc -c ../src/Encode.c -o ./object/Encode.o -I ../src

./object/Framer.o : ../src/Framer.c ../src/*.h
	cc -c ../src/Framer.c -o ./object/Framer.o -I ../src

.PHONY: clean
clean :
	rm $(target) $(objects)