CC=g++
CFLAGS=-c -s -Wall -O3 -march=native  -Werror -Wextra
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
