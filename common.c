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

char* tcpread_nbytes(int socket, int bytes){
    int bytes_left = bytes, bytes_read;
    char* buffer = malloc(sizeof(char)*(bytes+1));
    char* ptr;
    memset(buffer, '\0', bytes+1);
    ptr = &buffer[0];
    printf("reading %d bytes...\n", bytes);
    while( bytes_left > 0 ){
        bytes_read = read(socket, ptr, bytes_left);
        if(bytes_read < 0)
            DieWithError("Error reading from TCP connection. Qutting");
        bytes_left -= bytes_read;
        ptr += bytes_read;
    }
    printf("inside tcpread: %s \n", buffer);
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

char *get_time(int offset)
{
    time_t rawtime;
    printf("asda\n");
    struct tm * timeinfo;
    int len_time;
    char str_time[26];
    char *str_day_week, *str_month, *str_day, *str_hour, *str_min, *str_sec, *str_year;
    char* str_output = malloc(sizeof(char)*20);
    char aux[3];

    printf("asda\n");
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
    printf("wtf %s \n", str_output);
    printf("%p \n", str_output);
    return str_output;
}
/*returns -1 if curr_time > time_limit, 0 if equal, 1 if time_limit>curr_time*/

int compare_time(char * time_limit, char * curr_time){
    char* limit, curr;
    char* save_limit, save_curr;

    int result;

    printf("DEBUG: Broke %s", time_limit);
    limit = strtok_r(time_limit, "_", &save_limit);
    printf("into: %s\n", limit);

    printf("DEBUG: Broke %s", curr_time);
    curr = strtok_r(curr_time, "_", &save_curr);
    printf("into: %s\n", curr);


    result = compare_date(limit, curr);
    printf("DEBUG: compare_date returned: %d", result);
    if(result == - 1 || result == 1)
        return result;
    else{

        printf("DEBUG: Broke %s", time_limit);
        limit = strtok_r(NULL, "_", &save_limit);
        printf("into: %s\n", limit);

        printf("DEBUG: Broke %s", curr_time);
        curr = strtok_r(NULL, "_", &save_curr);
        printf("into: %s\n", curr);

        result = compare_hms(limit, curr);
        printf("DEBUG: compare_hms returned: %d", result);
        return result;
    }
}

/*returns -1 if curr > time, 0 if equal, 1 if limit>curr*/
int compare_date(char* time_limit, char* curr_time){
    char limit[5], curr[5];
    char* save_limit, save_curr;
    int result, i;

    /*years*/
    for(i=0; i<4; i++){
        limit[i] = time_limit[5+i];
    }
    limit[5+i] = "\0";
    printf("DEBUG: Broke %s into: %s\n", time_limit, limit);
    for(i=0; i<4; i++){
        curr[i] = curr_time[5+i];
    }
    curr[5+i] = "\0";
    printf("DEBUG: Broke %s into: %s\n", curr_time, curr);
    result = atoi(limit)-atoi(curr);
    if(result > 0)
        return 1;
    else if(result < 0)
        return -1;

    /*months*/
    for(i=0; i<3; i++){
        limit[i] = time_limit[2+i];
    }
    limit[2+i] = "\0";
    printf("DEBUG: Broke %s into: %s\n", time_limit, limit);
    for(i=0; i<3; i++){
        curr[i] = curr_time[2+i];
    }
    curr[2+i] = "\0";
    printf("DEBUG: Broke %s into: %s\n", curr_time, curr);
    result = month_to_int(limit)-month_to_int(curr);
    if(result > 0)
        return 1;
    else if(result < 0)
        return -1;

    /*days*/
    for(i=0; i<2; i++){
        limit[i] = time_limit[i];
    }
    limit[i] = "\0";
    printf("DEBUG: Broke %s into: %s\n", time_limit, limit);
    for(i=0; i<2; i++){
        curr[i] = curr_time[i];
    }
    curr[i] = "\0";
    printf("DEBUG: Broke %s into: %s\n", curr_time, curr);
    if(result > 0)
        return 1;
    else{
        if(result < 0)
            return -1;
    }

    return 0;
}

/*returns -1 if curr > time, 0 if equal, 1 if limit>curr*/
/*compare hour minute second*/
int compare_hms(char* time_limit, char* curr_time){
    char* limit, curr;
    char* save_limit, save_curr;
    int result, i;

    for(i=0; i<3; i++){
        limit = strtok_r(time_limit, ":", &save_limit);
        printf("DEBUG: Broke %s into: %s\n", time_limit, limit);
        curr = strtok_r(curr_time, ":", &save_limit);
        printf("DEBUG: Broke %s into: %s\n", curr_time, curr);
        result = atoi(limit)-atoi(curr);
        if(result == 1 || result == -1)
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

    if(strcmp("JAN", month)==0){
        printf("DEBUG: Returning JAN(1) for input: %s", month);
        return 1;
    }

    if(strcmp("FEB", month)==0){
        printf("DEBUG: Returning FEB(2) for input: %s", month);
        return 2;
    }
    if(strcmp("MAR", month)==0){
        printf("DEBUG: Returning MAR(3) for input: %s", month);
        return 3;
    }
    if(strcmp("APR", month)==0){
        printf("DEBUG: Returning APR(4) for input: %s", month);
        return 4;
    }
    if(strcmp("MAY", month)==0){
        printf("DEBUG: Returning MAY(5) for input: %s", month);
        return 5;
    }
    if(strcmp("JUN", month)==0){
        printf("DEBUG: Returning JUN(6) for input: %s", month);
        return 6;
    }
    if(strcmp("JUL", month)==0){
        printf("DEBUG: Returning JUL(7) for input: %s", month);
        return 7;
    }
    if(strcmp("AUG", month)==0){
        printf("DEBUG: Returning AUG(8) for input: %s", month);
        return 8;
    }
    if(strcmp("SEP", month)==0){
        printf("DEBUG: Returning SEP(9) for input: %s", month);
        return 9;
    }
    if(strcmp("OUT", month)==0){
        printf("DEBUG: Returning OUT(10) for input: %s", month);
        return 10;
    }
    if(strcmp("NOV", month)==0){
        printf("DEBUG: Returning NOV(11) for input: %s", month);
        return 11;
    }
    if(strcmp("DEC", month)==0){
        printf("DEBUG: Returning DEC(12) for input: %s", month);
        return 12;
    }

}

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

    if ((read = getline(&topic_number, &size, tfp)) != -1) {
        topic_number[strlen(topic_number)-1] = '\0';
    }


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

    out_string = (char *) malloc(sizeof(files[r]));

    strcpy(out_string, files[r]);
    out_string[strlen(out_string)-1] = '\0';

    fclose(ifp);
    fclose(tfp);
    return out_string;
}

