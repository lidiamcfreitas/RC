CC=gcc
CFLAGS=-c -Wall

all: user ecp tes mini_client 

user: user.o common.o
	$(CC) User.o common.o -o user

mini_client: mini_client.o DieWithError.o
	$(CC) mini_client.o common.o -lm -o mini_client

ecp: ecp.o DieWithError.o;
	$(CC) ECP.o common.o -o ECP

tes: tes.o DieWithError.o
	$(CC) TES.o common.o -o TES
    
user.o: dir_User/User.c
	$(CC) $(CFLAGS) dir_User/User.c

mini_client.o: dir_TES/mini_client.c
	$(CC) $(CFLAGS) dir_TES/mini_client.c

ecp.o: dir_ECP/ECP.c
	$(CC) $(CFLAGS) dir_ECP/ECP.c
    
tes.o: dir_TES/TES.c
	$(CC) $(CFLAGS) dir_TES/TES.c

common.o: common.c
	$(CC) $(CFLAGS) common.c

clean:
	rm *.o
