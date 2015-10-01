//
//  User.h
//  
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#ifndef ____User__
#define ____User__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define DEFAULT_PORT 58026 
#define MAX_TES 99
#define MAX_TOPIC_SIZE 25
void DieWithError(char *errorMessage);

char* tcpread_until_char(int socket, char c, int max_length, int terminate);

char* tcpread_nbytes(int socket, int bytes);

void tcpwrite(int socket, char* buffer, int nbytes); 

#endif /* defined(____User__) */
