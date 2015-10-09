/* common functions used in various files */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*Prints error message and exits */
void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

/*
  Reads from socket ( TCP file descriptor ) until character c is found, searching at maximum max_length bytes.
  Terminates the string on the character found if terminate is true
*/
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
            DieWithError("Error reading on TCP connection. Quitting\n");
        read_c = ptr[0];
        i++;
    }
    if(!terminate)
        ptr += bytes_read;
    ptr[0] = '\0';
    return buffer;
}
/*
  Reads from socket ( TCP file descriptor ) the number of bytes specified in argument 'bytes'.
  Returns unterminated string
*/
char* tcpread_nbytes(int socket, int bytes){
    int bytes_left = bytes, bytes_read;
    char* buffer = malloc(sizeof(char)*(bytes+1));
    char* ptr;
    memset(buffer, '\0', bytes+1);
    ptr = &buffer[0];
    while( bytes_left > 0 ){
        bytes_read = read(socket, ptr, bytes_left);
        if(bytes_read < 0)
            DieWithError("Error reading from TCP connection. Qutting");
        bytes_left -= bytes_read;
        ptr += bytes_read;
    }
    return buffer;
}

/*
  Writes nbytes bytes to socket (TCP fd) read from buffer
*/
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

/*
  Gets current time, with added offset, and returns a string with the necessary format
*/
char *get_time(int offset)
{
    time_t rawtime;
    struct tm * timeinfo;
    int len_time;
    char str_time[26];
    char *str_day_week, *str_month, *str_day, *str_hour, *str_min, *str_sec, *str_year;
    char* str_output = malloc(sizeof(char)*20);
    char aux[3];

    time ( &rawtime );
    rawtime += offset;
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
    str_output[19]='\0';
    // string format: DDMMMYYYY_HH:MM:SS
    return str_output;
}

/*
  Expects two date strings on speciefied format and compares them.
  Returns -1 if curr_time > time_limit, 0 if equal, 1 if time_limit>curr_time
*/
int compare_time(char * time_limit, char * curr_time){
    char* limit, *curr;
    char* save_limit, *save_curr;

    int result;

    limit = strtok_r(time_limit, "_", &save_limit);

    curr = strtok_r(curr_time, "_", &save_curr);

    result = compare_date(limit, curr);
    if(result == - 1 || result == 1)
        return result;
    else{

        limit = strtok_r(NULL, "_", &save_limit);
        curr = strtok_r(NULL, "_", &save_curr);
        result = compare_hms(limit, curr);
        return result;
    }
}

/*
  Helper function for compare_time
  returns -1 if curr > time, 0 if equal, 1 if limit>curr
*/
int compare_date(char* time_limit, char* curr_time){
    char limit[5], curr[5];
    char* save_limit, *save_curr;
    int result, i;

    /*years*/
    for(i=0; i<4; i++){
        limit[i] = time_limit[5+i];
    }
    limit[4] = '\0';

    for(i=0; i<4; i++){
        curr[i] = curr_time[5+i];
    }
    curr[4] = '\0';

    result = atoi(limit)-atoi(curr);
    if(result > 0)
        return 1;
    else if(result < 0)
        return -1;

    /*months*/
    for(i=0; i<3; i++){
        limit[i] = time_limit[2+i];
    }
    limit[3] = '\0';

    for(i=0; i<3; i++){
        curr[i] = curr_time[2+i];
    }
    curr[3] = '\0';

    result = month_to_int(limit)-month_to_int(curr);
    if(result > 0)
        return 1;
    else if(result < 0)
        return -1;

    /*days*/
    for(i=0; i<2; i++){
        limit[i] = time_limit[i];
    }
    limit[2] = '\0';

    for(i=0; i<2; i++){
        curr[i] = curr_time[i];
    }
    curr[2] = '\0';

    if(result > 0)
        return 1;
    else{
        if(result < 0)
            return -1;
    }

    return 0;
}

/*
  Helper function to compare hms
  returns -1 if curr > time, 0 if equal, 1 if limit>curr
*/
int compare_hms(char* time_limit, char* curr_time){
    char limit[3], curr[3];
    int limit_int, curr_int;
    int result, i;

    for(i=0; i<3; i++){

        limit[0] = time_limit[i*3+0];
        limit[1] = time_limit[i*3+1];
        limit[2] = '\0';
        limit_int = atoi(limit);

        curr[0] = curr_time[i*3+0];
        curr[1] = curr_time[i*3+1];
        curr[2] = '\0';
        curr_int = atoi(curr);

        result = limit_int - curr_int;
        if(result > 0 || result < 0)
            return result;
        }
    return result;
}

/*  JAN = 1
    FEB = 2
    MAR = 3
    APR = 4
    MAY = 5
    JUN = 6
    JUL = 7
    AUG = 8
    SEP = 9
    OUT = 10
    NOV = 11
    DEC = 12*/
int month_to_int(char* month){

    if(strcmp("Jan", month)==0){
        return 1;
    }

    if(strcmp("Feb", month)==0){
        return 2;
    }
    if(strcmp("Mar", month)==0){

        return 3;
    }
    if(strcmp("Apr", month)==0){

        return 4;
    }
    if(strcmp("May", month)==0){

        return 5;
    }
    if(strcmp("Jun", month)==0){

        return 6;
    }
    if(strcmp("Jul", month)==0){

        return 7;
    }
    if(strcmp("Aug", month)==0){

        return 8;
    }
    if(strcmp("Sep", month)==0){

        return 9;
    }
    if(strcmp("Oct", month)==0){

        return 10;
    }
    if(strcmp("Nov", month)==0){

        return 11;
    }
    if(strcmp("Dec", month)==0){
        
        return 12;
    }

}

/*
  Selects random file from questionnaires on folder
*/
char *random_file(){

    FILE *ifp, *tfp;
    char *mode = "r";
    char *file, *out_string;
    char files[100][13];
    int num_files, r, i;
    ssize_t read;
    size_t size;
    char *topic_number;

    num_files = 0;
    srand(time(NULL));

    system("ls ./dir_TES > dir_TES/ficheiros.txt");
    ifp = fopen("dir_TES/ficheiros.txt", mode);
    tfp = fopen("dir_TES/TES_number.txt", mode);

    if (ifp == NULL) {
        fprintf(stderr, "Can't open file with name of files\n");
        exit(1);
    }

    if (tfp == NULL){
        fprintf(stderr, "Can't open file with TES topic number.\n");
        exit(1);
    }
    topic_number = NULL;
    if ((read = getline(&topic_number, &size, tfp)) != -1) {
        topic_number[strlen(topic_number)-1] = '\0';
    }
    file = NULL;
    while ((read = getline(&file, &size, ifp)) != -1) {
        if((strncmp(file, topic_number, 3) == 0) && (file[strlen(file)-2]=='f')){ //only choose the .pdf files
            strcpy(files[num_files],file);
            num_files++;
        }
    }
    if (num_files == 0){
        printf("No .pdf questionnaires to show\n"); // TODO substitute with DieWithError
        exit(1);
    }

    r = rand() % (num_files);
    out_string = (char *) malloc(sizeof(files[r])+sizeof("dir_TES/")-sizeof(char));
    strcpy(out_string, "dir_TES/");
    strcat(out_string, files[r]);
    out_string[strlen(out_string)-1] = '\0';
    fclose(ifp);
    fclose(tfp);
    return out_string;
}
