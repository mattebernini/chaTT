
all: serv dev

dev: device.c 
	g++ –Wall device.c "lib/chatlib.c" -o dev

serv: server.c 
	g++ –Wall server.c "lib/chatlib.c" -o serv

clean:
	rm *o dev serv