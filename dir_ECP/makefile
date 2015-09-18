CC=gcc
CFLAGS=-c -Wall

all: ecp

ecp: ecp.o die.o
	$(CC) ecp.o -o ECP

ecp.o: ECP.c
	$(CC) $(CFLAGS) ECP.c