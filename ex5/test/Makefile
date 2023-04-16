all: main.o sci.o
	gcc -o app main.o sci.o

main.o: main.c sci.h
	gcc -c main.c

sci.o: sci.c sci.h
	gcc -c sci.c

clean:
	rm -rf app ./*.o