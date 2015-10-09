//
    //  TES.c
    //
    //
    //
    //

    #include "TES.h"
    #include "../CommonHeader.h"


    typedef struct{
        char* QID;
        int SID;
        char time_limit[19];
        int def;
    }user_info;
    user_info user_array[512];

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
        char *path_to_answer;
        char *path_to_file;
        struct hostent *ecphostptr;
        int i;
        pid_t pid;

        /*Initializing user array*/
        for(i = 0; i<99 ; i++){
            user_array[i].def = 0;
        }

        /* Argument parsing*/
        if( argc < 1 || argc > 7 || argc % 2 != 1 ) /* test for correct number of arguments */
        {
            fprintf(stderr, "Usage: %s [-p TESport] [-n ECPname] [-e ECPport]\n", argv[0]);
            exit(1);
        }

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

        /* End of argument parsing*/

        // UDP
        if ((udpsock_fd = socket(AF_INET, SOCK_DGRAM, 0))<0)
            DieWithError("UDP socket() failed");
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

        path_to_file = random_file();
        printf("Random pdf picked. Serving %s to users.\n", path_to_file);

        /* CHANGE */
        if((file_ptr = fopen(path_to_file, "r"))==NULL)
            DieWithError("pdf fopen() failed");
        path_to_answer = strtok(path_to_file, ".");
        strcat(path_to_answer, "A.txt");
        if((answers_ptr = fopen(path_to_answer, "r"))==NULL)
            DieWithError("answers fopen() failed");
        if((user_info_ptr = fopen("dir_TES/user_info.txt", "a+"))==NULL)
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
                printf("Child server created to handle request. Starting\n");
                process_request(new_fd);
                printf("Request finished. Closing child\n");
                break;
            }
            do
                i=close(new_fd);
            while(i==-1&&errno==EINTR);
            if(i == -1)
                DieWithError("Error closing socket on main TES");
        }
        printf("Closing one server instance\n");
        fclose(file_ptr);
        fclose(answers_ptr);
        close(udpsock_fd);
        close(sock_fd);
        close(new_fd);
        return 0;
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
        long file_size, block_size, bytes_sent, bytes_left;

        /*RQS*/
        if (strcmp(read_buffer, "RQS ") == 0){
            char* user_answer;
            char* getline_helper;
            char answers[5];
            char user_answers[5];
            char* QID;
            int SID, found = 0, score = 0;

            /*GET QID and SID from TCP*/
            read_buffer = tcpread_until_char(new_fd, '\n', 40, 1);

            i=0;


            /* read user_info_ptr*/
            char* temp= NULL;
            size_t len = 0;
            ssize_t read;
            rewind(user_info_ptr);
            while(read = getline(&temp,&len, user_info_ptr)!=-1){
                getline_helper = strtok(temp, " ");
                user_array[i].SID = atoi(getline_helper);

                getline_helper = strtok(NULL, " ");
                user_array[i].QID = malloc(sizeof(getline_helper));
                strcpy(user_array[i].QID,getline_helper);
                getline_helper = strtok(NULL, " ");
                strcpy(user_array[i].time_limit, getline_helper);
                user_array[i].def =1;

                i++;
            }
            user_answer = strtok(read_buffer, " ");
            SID = atoi(user_answer);
            user_answer = strtok(NULL, " ");
            printf("Parsing user answers\n");
            QID = malloc(sizeof(user_answer));
            strcpy(QID, user_answer);
            for(i=0; i<5 ;  i++){
                user_answer = strtok(NULL, " ");
                user_answers[i] = user_answer[0];
            }


            /* search user_array for received user */
            for(i=0; user_array[i].def != 0 && i< 99; i++ ){
                if(user_array[i].SID == SID && strcmp(user_array[i].QID,QID)==0){
                    printf("Match found on user: %d\n", user_array[i].SID);
                    printf("Testing time:%s\n", get_time());
                    if(compare_time(user_array[i].time_limit, get_time())<0)
                        score = -1;

                    found = 1;
                }
            }

            if(found == 0)
                score = -2;

            if(score == 0){
                /*Check answers and calculate score*/
                fscanf(answers_ptr, "%c\n%c\n%c\n%c\n%c", &answers[0],&answers[1],&answers[2],&answers[3],&answers[4]);
                printf("The right answers are %c %c %c %c %c\n", answers[0],answers[1],answers[2],answers[3],answers[4]);
                score = 0;
                for(i=0; i<5; i++){
                    if(user_answers[i] == answers[i]){
                        score += 20;
                    }
                    else{
                    }
                }
            }
            /*Preparing AQS response*/
            printf("Score calculated: %d / 100 \n", score);

            memset(write_buffer, '\0', 256);
            strcpy(write_buffer, "AQS ");
            memset(buffer, '\0', 32);
            strcat(write_buffer, QID);
            strcat(write_buffer, " ");
            memset(buffer, '\0', 32);
            sprintf(buffer, "%d", score);
            strcat(write_buffer, buffer);
            strcat(write_buffer, "\n");

            size_t message_size = strlen(write_buffer)*sizeof(char);
            printf("Sending %s", write_buffer);
            tcpwrite(new_fd, write_buffer, message_size);

            // UDP to ECP
            printf("Starting udp connection with ECP to report score\n");
            socklen_t addr_size;
            char rcv_buffer[30];
            char tosend_buffer[70];
            char aux_udp_ecp[40];
            FILE *name_fp;
            char *name;
            ssize_t udp_read;
            size_t udp_len;
            name = NULL;

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

            if ((udp_read = getline(&name, &udp_len, name_fp)) != -1) {
                name[strlen(name)-1] = '\0';
            }

            fclose(name_fp);
            strcat(tosend_buffer, " ");
            strcat(tosend_buffer, name);
            strcat(tosend_buffer, " ");
            sprintf(aux_udp_ecp, "%d", score);
            strcat(tosend_buffer, aux_udp_ecp); // add score
            strcat(tosend_buffer, "\n");
            printf("sending to ECP: %s", tosend_buffer);

            if(sendto(udpsock_fd, tosend_buffer, strlen(tosend_buffer), 0, (struct sockaddr*) &ecpAddr, sizeof(ecpAddr))<0)
                DieWithError("sendto() failed");

            addr_size = sizeof(ecpAddr);

            memset(&rcv_buffer, '\0', sizeof(rcv_buffer));
            if(((recvfrom(udpsock_fd, rcv_buffer, sizeof(rcv_buffer), 0, (struct sockaddr*) &ecpAddr, &addr_size))<0))
                DieWithError("recv() failed");
            if(strncmp(rcv_buffer, "AWI", 3) == 0){
                printf("Questionnaire submission successful\n");
            }else{
                printf("Can't recognize response from ECP.\n");
            }
    }
    /* RQT */
    else if(strcmp(read_buffer, "RQT ")==0){
        char* QID = malloc(sizeof(char)*24);
        long SID, QID_r;
        char* time_limit;
    
        /*GET SID*/
        read_buffer = tcpread_nbytes(new_fd, 5);
        SID = atoi(read_buffer);
        fprintf(user_info_ptr, "%d ", SID);
        printf("SID is: %d\n", SID);

        /*GENERATE QID*/
        srand (time(NULL));
        QID_r = rand() % 900000 + 100000;
        sprintf(QID, "%ld", QID_r);
        strcat(QID, "_");
        strcat(QID, read_buffer);
        fprintf(user_info_ptr, "%s ", QID);
        printf("Generated QID is: %s\n", QID);

        /*GET Current Time*/
        time_limit = get_time(600);
        fprintf(user_info_ptr, "%s\n", time_limit);
        fflush(user_info_ptr);
        printf("Time limit is: %s\n", time_limit);

        printf("Assessing file size...\n");
        /*Finding file size*/
        fseek(file_ptr, 0, SEEK_END);
        file_size = ftell(file_ptr);
        rewind(file_ptr);

        /*Preparing AQT response*/
        strcpy(write_buffer, "AQT ");
        strcat(write_buffer, QID);
        strcat(write_buffer, " ");
        strcat(write_buffer, time_limit);
        strcat(write_buffer, " ");
        sprintf(buffer, "%d", file_size);
        strcat(write_buffer, buffer);
        strcat(write_buffer, " ");

        size_t message_size = strlen(write_buffer)*sizeof(char);
        printf("Sending message before file transfer: %s , size %d, file_size: %d\n", write_buffer, message_size,file_size);
        tcpwrite(new_fd, write_buffer, message_size);

        /*Streaming file through TCP*/
        bytes_left = file_size;
        memset(write_buffer, '\0', 256);
        ptr = &write_buffer[0];
        printf("Sending file. %d bytes left\n", bytes_left);
        while(bytes_left > 0){
            block_size = fread(write_buffer, sizeof(char), 256, file_ptr);
            bytes_left -= block_size;
            tcpwrite(new_fd, write_buffer, block_size);
        }
        strcpy(write_buffer, "\n");
        tcpwrite(new_fd, write_buffer, 1);
        printf("File sent successfuly!\n");
        sleep(1);
    }
    else{
        printf("Can't Recognize Transmission");
    }


}
