CC=gcc
CFLAGS=
RM=rm -rf
OUT1=server
OUT2=C

all: server client

server: server.o 
	$(CC) $(CFLAGS) -o $(OUT1) Server-MCH.c

client:	SocketsClient.o
	$(CC) -o ${OUT2} SocketsClient.c

debug: CFLAGS+=-DDEBUG_ON
debug: build

server.o: Server-MCH.c 
	$(CC) $(CFLAGS) -c Server-MCH.c

SocketsClient.o: SocketsClient.c
	$(CC) -c SocketsClient.c

clean:
	$(RM) *.o $(OUT)
