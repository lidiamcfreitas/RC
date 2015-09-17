CC=gcc
CFLAGS=-c -Wall

all: user ecp tes

user: user.o die.o
	$(CC) user.o DieWithError.o -o user

ecp: ecp.o
	$(CC) ecp.o -o ECP

tes: tes.o
	$(CC) tes.o -o TES
    
user.o: User.c
	$(CC) $(CFLAGS) User.c

ecp.o: ECP.c
	$(CC) $(CFLAGS) ECP.c
    
tes.o: TES.c
	$(CC) $(CFLAGS) TES.c


die.o: DieWithError.c
	$(CC) $(CFLAGS) DieWithError.c

clean:
	rm *.o