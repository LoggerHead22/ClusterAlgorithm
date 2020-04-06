CFLAGS = -W -Wall -Werror
CC = g++

all : tcp 

tcp : cli_tcp ser_tcp


cli_tcp : cli_tcp.o
	$(CC) $(CFLAGS) cli_tcp.o -o cli_tcp

cli_tcp.o : cli_tcp.cpp
	$(CC) $(CFLAGS) -c cli_tcp.cpp -o cli_tcp.o

ser_tcp : ser_tcp.o
	$(CC) $(CFLAGS) ser_tcp.o -o ser_tcp

ser_tcp.o : ser_tcp.cpp
	$(CC) $(CFLAGS) -c ser_tcp.cpp -o ser_tcp.o




clean :
	rm *.o cli_tcp ser_tcp 
