all: dev serv

#
dev: device.o ./lib/utility.o ./lib/netlib.o
	gcc -Wall -pthread device.o ./lib/utility.o ./lib/netlib.o -o dev

serv: server.o ./lib/utility.o ./lib/netlib.o
	gcc -Wall server.o ./lib/utility.o ./lib/netlib.o -o serv

#
device.o: device.c 
	gcc -c -Wall -pthread device.c -o device.o

server.o: server.c 
	gcc -c -Wall server.c -o server.o

#
utility.o: utility.c 
	gcc -c -Wall utility.c -o utility.o

netlib.o: netlib.c 
	gcc -c -Wall netlib.c -o netlib.o

# elimina .o exe e chat
clean: 
	rm *.o dev serv ./lib/*.o ./device/*.txt