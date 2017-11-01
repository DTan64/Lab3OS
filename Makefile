CC=gcc
CFLAGS= -lpthread -O2

all:
	gcc -o multiThread multi-lookup.c util.c
clean:
	rm result.txt
	rm serviced.txt
	rm multiThread
