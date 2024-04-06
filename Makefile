all: world
CXX?=g++
CXXFLAGS?=--std=c++17 -Wall -fPIC -g
LDFLAGS?=-L/lib -L/usr/lib

INCLUDES+= -I./include

OBJS:= \
	objs/main.o

PROCESS_DIR:=.
include throws/Makefile.inc
include Makefile.inc

world: example

$(shell mkdir -p objs)

objs/main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

example: $(THROWS_OBJS) $(PROCESS_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -L. $(LIBS) $^ -o $@;

.PHONY: clean
clean:
	rm -f objs/*.o example
