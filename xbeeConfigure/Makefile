PACKAGE=xbeeConfigure
CFLAGS=-Wall
LDFLAGS=-ggdb -Wall -pthread
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
