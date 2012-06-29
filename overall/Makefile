PACKAGE=overall
CFLAGS=-Wall
LDFLAGS=-ggdb -Wall `pkg-config --cflags opencv`
LDLIBS=`pkg-config --libs opencv`
OBJECTS=rs232.o

all: $(PACKAGE)

$(PACKAGE): $(OBJECTS)

serial: rs232.o
	g++ -Wall -ggdb -o serialReceive serialReceive.cpp rs232.o

rs232.o: rs232.h

.PHONY: run clean cleanall

run: all
	./$(PACKAGE)

clean:
	-rm $(OBJECTS)

cleanall: clean
	-rm $(PACKAGE)
