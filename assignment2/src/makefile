CC=g++
CFLAGS=-std=c++11


all: main test install

main: LoC.o
	$(CC) $(CFLAGS) -o main main.cpp LoC.o

LoC.o: LoC.cpp
	$(CC) $(CFLAGS) -c LoC.cpp

test: LoC.o
	$(CC) $(CFLAGS) -o ../test/test1 ../test/test1.cpp LoC.o

clean:
	rm -rf ../lib/*.o 
	rm -rf ../bin/main
	rm -rf ../test/test1

install:
	mv *.o ../lib/
	mv main ../bin/main

