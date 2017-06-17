CC=g++
CFLAGS=-c -Wall -O3
LDFLAGS=
SOURCES=memcpy.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=memcpy

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ 

run: 
	./$(EXECUTABLE)
clear:
	rm *.o
