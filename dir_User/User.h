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

void DieWithError(char *errorMessage);

#endif /* defined(____User__) */
