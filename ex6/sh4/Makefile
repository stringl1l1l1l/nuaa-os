all: main.o parse.o cmd.o
	cc -o sh4 main.o parse.o cmd.o

main.o: main.c parse.h cmd.h
	cc -c main.c

parse.o: define.h parse.c parse.h
	cc -c parse.c

cmd.o: define.h cmd.c cmd.h
	cc -c cmd.c

clean: 
	rm -rf sh4 *.o
