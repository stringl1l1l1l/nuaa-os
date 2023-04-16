all: cmd.o parse.o main.o
	gcc -g -o sh3 cmd.o parse.o main.o

parse.o: parse.c parse.h define.h cmd.h
	gcc -g -c parse.c

cmd.o: cmd.c cmd.h define.h
	gcc -g -c cmd.c

main.o: main.c cmd.h define.h parse.h
	gcc -g -c main.c

clean:
	rm -rf sh3 ./*.o