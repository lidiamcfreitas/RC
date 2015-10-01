#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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
        bytes_left -= bytes_written;
        ptr += bytes_written; 
    }
}

char *get_time()
{
    time_t rawtime;
    struct tm * timeinfo;
    int len_time;
    char str_time[26];
    char *str_day_week, *str_month, *str_day, *str_hour, *str_min, *str_sec, *str_year;
    char* str_output = (char *) malloc(19);
    char aux[3];

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strcpy(str_time, asctime(timeinfo));

    
    str_day_week = strtok(str_time, " :");
    str_month = strtok(NULL, " :");
    str_day = strtok(NULL, " :");
    str_hour = strtok(NULL, " :");
    str_min = strtok(NULL, " :");
    str_sec = strtok(NULL, " :");
    str_year = strtok(NULL, " :\n");

    
    if (strlen(str_day)==1){
        strcpy(aux, "0");
        strcat(aux, str_day);
        str_day = aux;
    }

    strcpy(str_output, str_day);
    strcat(str_output, str_month);
    strcat(str_output, str_year);
    strcat(str_output, "_");
    strcat(str_output, str_hour);
    strcat(str_output, ":");
    strcat(str_output, str_min);
    strcat(str_output, ":");
    strcat(str_output, str_sec);
     
    // string format: DDMMMYYYY_HH:MM:SS

    return str_output;
}

