INCLUDE = -I/usr/local/include

CXX      = g++ -std=c++11
CFLAGS   = -Wall -g -O2 $(INCLUDE) -MD
CPPFLAGS = $(CFLAGS)

LIBS     =
LIBS    += -lrabbitmq
LIBS    += -lamqp11
LIBS    += -lboost_thread

SOURCES  = consumer.cpp SynchronizedQueue.cpp
OBJECTS  = $(SOURCES:.cpp=.o)

PROGS   = consumer SynchronizedQueue

all: $(PROGS)

#consumer: $(OBJECTS)
consumer: consumer.o
	g++ -o $@ $^ $(LIBS)

SynchronizedQueue: SynchronizedQueue.o
	g++ -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJECTS)
