CC 	:= gcc
CFLAGS  := -O -Wall -Wno-unused -Wno-unused-result -Wno-parentheses -Wundef

all:
	$(CC) $(CFLAGS) -lpthread -o server server.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client

restart: kill run

kill:
	pkill -9 server

run:
	./server&
	./client aaa&
	./client bbb&
	./client ccc&