CC=clang
IDIR=./include
BDIR=./build
TARGET=main
CFLAGS=-std=c99 -Wall -pedantic -I $(IDIR) -L -lruntime
SRC=$(wildcard src/*.c)
OBJ=$(SRC:src/%.c=build/%.o)

all: mkbuild $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

build/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

rebuild: clean mkbuild all

mkbuild:
	mkdir -p build

clean:
	rm -rf build main
