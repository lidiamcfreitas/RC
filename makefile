CC=gcc
CFLAGS=-w -c -Wall

all: user ecp tes 

user: user.o common.o
	$(CC) User.o common.o -o user

ecp: ecp.o DieWithError.o;
	$(CC) ECP.o common.o -o ECP

tes: tes.o DieWithError.o
	$(CC) TES.o common.o -o TES
    
user.o: dir_User/User.c
	$(CC) $(CFLAGS) dir_User/User.c

ecp.o: dir_ECP/ECP.c
	$(CC) $(CFLAGS) dir_ECP/ECP.c
    
tes.o: dir_TES/TES.c
	$(CC) $(CFLAGS) dir_TES/TES.c

common.o: common.c
	$(CC) $(CFLAGS) common.c

clean:
	rm *.o
