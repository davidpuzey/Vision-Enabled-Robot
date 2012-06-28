PACKAGE=overall
CFLAGS=-Wall
LDFLAGS=-ggdb -Wall `pkg-config --cflags opencv`
LDLIBS=`pkg-config --libs opencv`
OBJECTS=rs232.o

all: $(PACKAGE)

$(PACKAGE): $(OBJECTS)

rs232.o: rs232.h

.PHONY: run clean cleanall

run: all
	./$(PACKAGE)

clean:
	-rm $(OBJECTS)

cleanall: clean
	-rm $(PACKAGE)
