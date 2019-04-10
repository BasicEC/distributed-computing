CC=clang
TARGET=main
CFLAGS=-std=c99 -Wall -pedantic

all: 
	$(CC) -o $(TARGET) $(CFLAGS) *.c -L -lruntime libruntime.so

clean:
	rm -rf build main
