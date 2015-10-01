#include <stdio.h>
#include <stdlib.h>

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

char* tcpread_until_char(int socket, char c, int max_length, int terminate){
    char* buffer = malloc(sizeof(char)*max_length);
    char* ptr; 
    int bytes_read; 
    memset(buffer, '\0', max_length); 
    ptr = &buffer[0];
    bytes_read = read(socket, ptr, 1);
    char read_c = ptr[0];
    int i = 0;
    while( (read_c != c ) && i < max_length){
        ptr += bytes_read;
        bytes_read = read(socket, ptr, 1);
        if(bytes_read<0)
            DieWithError("Error reading on TCP connection. Qutting\n");
        read_c = ptr[0];
        i++;
    }
    if(!terminate)
        ptr += bytes_read;
    ptr[0] = '\0';
    return buffer;
}

char* tcpread_nbytes(int socket, int bytes){
    int bytes_left = bytes, bytes_read;
    char* buffer = malloc(sizeof(char)*bytes);
    char* ptr; 
    memset(buffer, '\0', bytes);
    ptr = &buffer[0];
    printf("reading %d bytes...\n", bytes);
    while( bytes_left > 0 ){
        bytes_read = read(socket, ptr, bytes_left);
        if(bytes_read < 0)
            DieWithError("Error reading from TCP connection. Qutting");
        bytes_left -= bytes_read;
        ptr += bytes_read; 
    }
    return buffer;
}

void tcpwrite(int socket, char* buffer, int nbytes){ 
    char* ptr;
    int bytes_left = nbytes, bytes_written;
    ptr = &buffer[0];
    while(bytes_left > 0){
        bytes_written = write(socket, ptr, bytes_left);
        if(bytes_written<0)
            DieWithError("Error writing to TCP connection. Quitting");
        bytes_left -= bytes_written;
        ptr += bytes_written; 
    }
}
