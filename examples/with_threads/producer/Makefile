INCLUDE = -I/usr/local/include

CXX      = g++ -std=c++11
CFLAGS   = -Wall -g -O2 $(INCLUDE) -MD
CPPFLAGS = $(CFLAGS)

LIBS     =
LIBS    += -lrabbitmq
LIBS    += -lamqp11
LIBS    += -lboost_thread

SOURCES  = producer.cpp
OBJECTS  = $(SOURCES:.cpp=.o)

PROGS   = producer

all: $(PROGS)

producer: $(OBJECTS)
	g++ -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJECTS)
