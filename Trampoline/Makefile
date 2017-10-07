CC=g++
CFLAGS=-c -s -Wall -O3 -march=native  -Werror -Wextra
LDFLAGS=
SOURCES=allocator.hpp allocator.cpp trampoline.hpp main.cpp argument_types.hpp instructions.hpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=trampoline_test

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ 

run: 
	./$(EXECUTABLE)
clear:
	rm *.o
