CC=gcc
CFLAGS= -lpthread -O2

all:
	gcc -o multi-lookup multi-lookup.c util.c
clean:
	rm result.txt
	rm serviced.txt
	rm multiThread
