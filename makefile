CC=gcc
CFLAGS=-c -Wall

all: user ecp tes mini_client

user: user.o DieWithError.o
	$(CC) User.o DieWithError.o -o user

mini_client: mini_client.o DieWithError.o
	$(CC) mini_client.o DieWithError.o -lm -o mini_client

ecp: ecp.o DieWithError.o;
	$(CC) ECP.o DieWithError.o -o ECP

tes: tes.o DieWithError.o
	$(CC) TES.o DieWithError.o -o TES
    
user.o: dir_User/User.c
	$(CC) $(CFLAGS) dir_User/User.c

mini_client.o: dir_TES/mini_client.c
	$(CC) $(CFLAGS) dir_TES/mini_client.c

ecp.o: dir_ECP/ECP.c
	$(CC) $(CFLAGS) dir_ECP/ECP.c
    
tes.o: dir_TES/TES.c
	$(CC) $(CFLAGS) dir_TES/TES.c

DieWithError.o: DieWithError.c
	$(CC) $(CFLAGS) DieWithError.c

clean:
	rm *.o
