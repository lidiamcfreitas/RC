CC=gcc
CFLAGS=-c -Wall

all: user ecp tes clean

user: user.o DieWithError.o
	$(CC) user.o DieWithError.o -o user

ecp: ecp.o DieWithError.o
	$(CC) ecp.o DieWithError.o -o ECP

tes: tes.o
	$(CC) tes.o -o TES
    
user.o: dir_User/User.c
	$(CC) $(CFLAGS) dir_User/User.c

ecp.o: dir_ECP/ECP.c
	$(CC) $(CFLAGS) dir_ECP/ECP.c
    
tes.o: dir_TES/TES.c
	$(CC) $(CFLAGS) dir_TES/TES.c


DieWithError.o: DieWithError.c
	$(CC) $(CFLAGS) DieWithError.c

clean:
	rm *.o