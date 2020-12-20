CC=gcc

CFLAGS=-Werror -pedantic -O2 -fPIC -lm

TARGETS=libmemory.so testoptimal testsimple testfusion testdivision

all: $(TARGETS)

libmemory.so: memory.h memory.o
	$(CC) -shared -fPIC -o libmemory.so memory.o

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c

testoptimal : testoptimal.c memory.o
	$(CC) -o testoptimal testoptimal.c memory.o

testsimple : testsimple.c memory.o
	$(CC) -o testsimple testsimple.c memory.o

testfusion : testfusion.c memory.o
	$(CC) -o testfusion testfusion.c memory.o

testdivision : testdivision.c memory.o
	$(CC) -o testdivision testdivision.c memory.o

