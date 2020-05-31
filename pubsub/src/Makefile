CC=g++
CFLAGS=-I. -g -std=c++11 -lpthread
DEPS = ../header/pubsub.h ../header/connection.h ../header/message.h
OBJ = pubsub.o connection.o main.o 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

apllication: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
.PHONY: clean

clean:
	rm -f *.o  	
