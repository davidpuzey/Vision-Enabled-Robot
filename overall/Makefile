CC=g++
CFLAGS=-ggdb -Wall `pkg-config --cflags opencv`
DEPS=rs232.c
LIBS=`pkg-config --libs opencv`

all: rs232.o
	g++ $(CFLAGS) overall.cpp rs232.o $(LIBS) -o overall

rs232.o: rs232.c rs232.h
	g -c -Wall rs232.c

clean:
	rm -rf *.o

cleanall:
	rm -rf *.o overall
