CC=g++
CFLAGS=-c -s -Wall -O3 -msse4.1
LDFLAGS=
SOURCES=word_counter.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=word_counter

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ 

run: 
	./$(EXECUTABLE)
clear:
	rm *.o
