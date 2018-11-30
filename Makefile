CC=gcc
CFLAGS=
RM=rm -rf
OUT1=server
OUT2=client

all: server client

server: server.o 
	$(CC) $(CFLAGS) -o $(OUT1) server.c

client: client.o
	$(CC) -o ${OUT2} client.c

debug: CFLAGS+=-DDEBUG_ON
debug: build

server.o: server.c 
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) -c client.c

clean:
	$(RM) *.o $(OUT1) $(OUT2)
