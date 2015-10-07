//
//  TES.c
//
//
//
//

#include "TES.h"
#include "../CommonHeader.h"


typedef struct{
    long QID;
    int SID;
    char time_limit[19];
    int def;
}user_info;
user_info user_array[99];

FILE *file_ptr;
FILE *answers_ptr;
FILE *user_info_ptr;

struct sockaddr_in server_addr, client_addr;
struct sockaddr_in ecpAddr;
int udpsock_fd;

int main(int argc, char *argv[]){

    int sock_fd;
    int new_fd;
    unsigned int client_addr_len;
    unsigned short server_port;
    unsigned short ecp_port;
    unsigned short topic_port;
    struct hostent *ecphostptr;
    int i;
    pid_t pid;

    for(i = 0; i<99 ; i++){
        user_array[i].def = 0;
    }

    if( argc < 1 || argc > 7 || argc % 2 != 1 ) /* test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s [-p TESport] [-n ECPname] [-e ECPport]\n", argv[0]);
        exit(1);
    }

    printf("%d\n", argc);
    if(argc==7){
        server_port = atoi(argv[2]);
        if((ecphostptr = gethostbyname(argv[4]))<0)
            DieWithError("gethostbyname() failed");
        ecp_port = atoi(argv[6]);

    } else if(argc == 3 && (strcmp(argv[1], "-p") == 0)){
        server_port = atoi(argv[2]);
        if((ecphostptr = gethostbyname("localhost"))<0)
            DieWithError("gethostbyname() failed");
        ecp_port = DEFAULT_PORT;

    } else if(argc == 3 && (strcmp(argv[1], "-n") == 0)){
        ecp_port = DEFAULT_PORT;
        server_port = 59000;
        if((ecphostptr = gethostbyname(argv[2]))<0)
            DieWithError("gethostbyname() failed");

    } else if(argc == 3 && (strcmp(argv[1], "-e") == 0)){
        server_port = 59000;
        if((ecphostptr = gethostbyname("localhost"))<0)
            DieWithError("gethostbyname() failed");
        server_port = atoi(argv[2]);

    } else if(argc == 5 && (strcmp(argv[1], "-p") &&  (strcmp(argv[3], "-n") == 0))){
        server_port = atoi(argv[2]);
        if((ecphostptr = gethostbyname(argv[4]))<0)
            DieWithError("gethostbyname() failed");
        ecp_port = DEFAULT_PORT;

    } else if(argc == 5 && (strcmp(argv[1], "-p") &&  (strcmp(argv[3], "-e")  == 0))){
        server_port = atoi(argv[2]);
        if((ecphostptr = gethostbyname("localhost"))<0)
            DieWithError("gethostbyname() failed");
        server_port = atoi(argv[4]);

    } else if(argc == 5 && (strcmp(argv[1], "-n") &&  (strcmp(argv[3], "-e")  == 0))){
        server_port = 59000;
        if((ecphostptr = gethostbyname(argv[2]))<0)
            DieWithError("gethostbyname() failed");
        server_port = atoi(argv[4]);

    } else {
        server_port = 59000;
        if((ecphostptr = gethostbyname("localhost"))<0)
            DieWithError("gethostbyname() failed");
        ecp_port = DEFAULT_PORT;
    }

    printf("DEBUG: server_port=%d\n", server_port);
    printf("DEBUG: ecp_port=%d\n", ecp_port);

    // UDP
    if ((udpsock_fd = socket(AF_INET, SOCK_DGRAM, 0))<0)
        DieWithError("UDP socket() failed");
    printf("udp: 1\n");
    /* define server address structure */
    memset(&ecpAddr, '\0', sizeof(ecpAddr));
    ecpAddr.sin_family = AF_INET;
    ecpAddr.sin_addr.s_addr = ((struct in_addr*)(ecphostptr->h_addr_list[0]))->s_addr;
    ecpAddr.sin_port = htons(ecp_port);


    // TPC
    if((sock_fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
        DieWithError("socket() failed");

    /* define server address structure */
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if(bind(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr))<0)
        DieWithError("bind() failed");


    /* CHANGE */

    char *path_to_file;
    path_to_file = random_file();
    printf("%s\n", path_to_file);

    /* CHANGE */
    if((file_ptr = fopen(path_to_file, "r"))==NULL)
        DieWithError("pdf fopen() failed");
    if((answers_ptr = fopen("dir_TES/answers.txt", "r"))==NULL)
        DieWithError("answers fopen() failed");
    if((user_info_ptr = fopen("dir_TES/user_info.txt", "a"))==NULL)
        DieWithError("user_info fopen() failed");

    listen(sock_fd, 5);
    for(;;){
        client_addr_len = sizeof(client_addr);
        printf("TESP: waiting for connection\n");
        do 
            new_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len); 
        while( new_fd == -1 && errno == EINTR);
        if(new_fd == -1)
            DieWithError("Failed to establish connection");
        if((pid=fork()) == -1)
            DieWithError("Error creating child proccess");
        else if(pid==0){
            printf("(Debug)Child server created to handle request. Starting\n");
            process_request(new_fd);
            printf("(Debug)Request finished. Closing child\n");
            break;
        }
        printf("plz\n");
        do 
            i=close(new_fd);
        while(i==-1&&errno==EINTR);
        if(i == -1)
            DieWithError("Error closing socket on main TES");
    }
    printf("(Debug)closing parent\n");
    fclose(file_ptr);
    fclose(answers_ptr);
    close(udpsock_fd);
    close(sock_fd);
    close(new_fd);
}

void process_request(int new_fd){ 
    char* read_buffer;
    char write_buffer[256];
    char buffer[32];
    char* command;
    char* ptr;
    int i;

    read_buffer = tcpread_nbytes(new_fd, 4);
    printf("Received %s From %s:%d\n", read_buffer, inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    printf("Command: %s\n", read_buffer); 
    long file_size, block_size, bytes_sent, bytes_left;

    /*RQS*/
    if (strcmp(read_buffer, "RQS ") == 0){
        char* user_answer;
        char answers[4];
        long QID;
        int SID, found = 0, score;

        /*GET QID and SID from TCP*/
        read_buffer = tcpread_until_char(new_fd, '\n', 40, 1);
        printf("(DEBUG)Received TCP: %s\n", read_buffer);
        user_answer = strtok(read_buffer, " ");
        QID = atol(user_answer);
        user_answer = strtok(NULL, " ");
        SID = atoi(user_answer);

        i=0;
        /* read user_info_ptr*/
        while(fscanf(user_info_ptr, "%ld %d %s", &user_array[i].QID, &user_array[i].SID, user_answer)==3){
            strcpy(user_array[i].time_limit, user_answer);
            user_array[i].def = 1;
            i++;
        }
        /* search user_array for received user */
        for(i=0; user_array[i].def != 0 && i< 99; i++ ){
            if(strcmp(user_array[i].SID, SID)==0 && strcmp(user_array[i].QID, QID)==0){
                printf("DEBUG: Match found on user: %d", user_array[i].SID);
                if(compare_time(user_array[i].time_limit, get_time())<0)
                    DieWithError("Time limit exceeded");
                found = 1;
            }
        }
        /*FIX ME score -1 se timeout -2 else normal*/
        //if(found == 0)
        //  DieWithError("User SID not found");

        /*check answers and calculate score*/
        fscanf(answers_ptr, "%c %c %c %c %c", &answers[0],&answers[1],&answers[2],&answers[3],&answers[4]);
        printf("The right answers are %c %c %c %c %c", answers[0],answers[1],answers[2],answers[3],answers[4]);        
        score = 0;
        for(i=0; i<5; i++){
            user_answer = strtok(NULL, " ");
            if(user_answer[0] == answers[i]){
                printf("Right answer on %d\n", i);
                score += 20;
            }
            else{
                printf("Wrong answer on %d\n", i);
            }
        }
        printf("Score calculated: %d\n", score);
        memset(write_buffer, '\0', 256);
        strcpy(write_buffer, "AQS ");
        memset(buffer, '\0', 32);
        sprintf(buffer, "%ld", QID);
        printf("TEST: %s\n", buffer);
        strcat(write_buffer, buffer);
        strcat(write_buffer, " ");
        memset(buffer, '\0', 32);
        sprintf(buffer, "%d", score);
        printf("TEST: %s\n", buffer);
        strcat(write_buffer, buffer);
        strcat(write_buffer, "\n");
        size_t message_size = strlen(write_buffer)*sizeof(char);
        printf("Sending %s", write_buffer);
        tcpwrite(new_fd, write_buffer, message_size);

        // UDP to ECP
        socklen_t addr_size;
        char rcv_buffer[30];
        char tosend_buffer[70];
        char aux_udp_ecp[6];
        FILE *name_fp;
        char *name;
        size_t size;
        ssize_t read;

        sprintf(aux_udp_ecp, "%d", SID);
        strcpy(tosend_buffer, "IQR ");
        strcat(tosend_buffer, aux_udp_ecp);
        strcat(tosend_buffer, " ");
        strcat(tosend_buffer, QID);

        name_fp = fopen("dir_TES/TES_name.txt", "r");

        if (name_fp == NULL) {
            fprintf(stderr, "Can't get topic name from TES_name\n");
            exit(1);
        }

        if ((read = getline(&name, &size, name_fp)) != -1) {
            name[strlen(name)-1] = '\0';
        }

        fclose(name_fp);

        strcat(tosend_buffer, name);
        strcat(tosend_buffer, " ");
        strcat(tosend_buffer, buffer); // add score
        strcat(tosend_buffer, '\0');
        
        //if(sendto(sock_fd, "IQR 12345 QID_cena topic_name 100\n", strlen("IQR 12345 QID_cena topic_name 100\n"), 0, (struct sockaddr*) &servAddr, sizeof(servAddr))<0)
        if(sendto(udpsock_fd, tosend_buffer, strlen(tosend_buffer), 0, (struct sockaddr*) &ecpAddr, sizeof(ecpAddr))<0)
            DieWithError("sendto() failed");
        
        addr_size = sizeof(ecpAddr);

        if(((recvfrom(udpsock_fd, rcv_buffer, sizeof(rcv_buffer), 0, (struct sockaddr*) &ecpAddr, &addr_size))<0))  
            DieWithError("recv() failed");
        printf("%s\n", rcv_buffer);

    }
    /* RQT */
    else if(strcmp(read_buffer, "RQT ")==0){
        long QID, SID;
        char* time_limit;
        printf("(DEBUG) Processing RQT\n");

        /*GET SID*/
        read_buffer = tcpread_nbytes(new_fd, 5);
        SID = atoi(read_buffer);
        fprintf(user_info_ptr, "%d ", SID);
        printf("DEBUG: SID is: %d\n", SID);

        /*GENERATE QID*/
        srand (time(NULL));
        QID = rand() % 900000 + 100000;
        fprintf(user_info_ptr, "%ld ", QID);
        printf("DEBUG: QID is: %ld\n", QID);

        /*GET Current Time*/
        time_limit = get_time(600);
        fprintf(user_info_ptr, "%s\n", time_limit);
        printf("DEBUG: Time limit is: %s\n", time_limit);
        fclose(user_info_ptr);

        printf("(Debug) Assessing file size\n");
        /*Transmission Stuff*/
        fseek(file_ptr, 0, SEEK_END);
        file_size = ftell(file_ptr);
        rewind(file_ptr);
        printf("(Debug) Preparing AQT response\n");
        strcpy(write_buffer, "AQT ");
        sprintf(buffer, "%ld", QID);
        strcat(write_buffer, buffer);
        strcat(write_buffer, " ");
        strcat(write_buffer, time_limit);
        strcat(write_buffer, " ");
        sprintf(buffer, "%d", file_size);
        strcat(write_buffer, buffer);
        strcat(write_buffer, " ");
        size_t message_size = strlen(write_buffer)*sizeof(char);
        printf("Sending %s , size %d, file_size: %d\n", write_buffer, message_size,file_size);
        tcpwrite(new_fd, write_buffer, message_size);

        /*Copiar código, mandar write_buffer pelo socket*/
        bytes_left = file_size;
        memset(write_buffer, '\0', 256);
        ptr = &write_buffer[0];
        printf("Sending file. %d bytes left\n", bytes_left);
        while(bytes_left > 0){
            block_size = fread(write_buffer, sizeof(char), 256, file_ptr);
            bytes_left -= block_size;
            tcpwrite(new_fd, write_buffer, block_size); 
            //printf("%d bytes left\n", bytes_left);
            /* while(block_size > 0){
               bytes_sent = write(new_fd, ptr, block_size);
               if(write < 0){
               printf("Error sending file");
               break;
               }
               block_size -= bytes_sent;
               ptr += bytes_sent;
               }
               ptr = &write_buffer[0];*/
            //            printf("Sending file... %d bytes sent\n", file_size-bytes_left);
        }
        strcpy(write_buffer, "\n");
        tcpwrite(new_fd, write_buffer, 1);
        printf("0 bytes left.\nFile sent successfuly!\n");
        sleep(1);
    }
    else{
        printf("Can't Recognize Transmission");
    }


}
