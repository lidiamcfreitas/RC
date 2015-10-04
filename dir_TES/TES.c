//
//  TES.c
//
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#include "TES.h"
#include "../CommonHeader.h"

void retrieve_file(char* file){
   FILE* file_ptr;
    if((file_ptr = fopen("dir_ECP/topics.txt", "r"))==NULL)
        DieWithError("fopen() failed");
    fclose(file_ptr);
}

int main(int argc, char *argv[]){

    int sock_fd;
    int new_fd;
    struct sockaddr_in server_addr, client_addr;
    unsigned int client_addr_len;
    unsigned short server_port;
    unsigned short ecp_port;
    int recv_string_len;
    FILE *file_ptr;
    FILE *answers_ptr;
    FILE *user_info_ptr;
    unsigned short topic_port;
    char* ecp_name;
    char* read_buffer;
    char write_buffer[256];
    char buffer[32];
    char* command;

    typedef struct{
        long QID;
        int SID;
        char time_limit[19];
    }user_info;

    user_info user_array[99];
    if(argc==5){
        server_port = atoi(argv[1]);
        ecp_name = argv[2];
        ecp_port = atoi(argv[3]);
    } else if(argc == 3 && (strcmp(argv[1], "-p") == 0)){
        server_port = atoi(argv[2]);
    } else {
    	fprintf(stderr, "Usage: %s [-p ECPname]\n", argv[0]);
    	exit(1);
     }

    if((sock_fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
       DieWithError("socket() failed");

    /* define server address structure */
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if(bind(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr))<0)
    	DieWithError("bind() failed");

    if((file_ptr = fopen("dir_ECP/topics.pdf", "r"))==NULL)
        DieWithError("topics fopen() failed");
    if((answers_ptr = fopen("dir_TES/answers.txt", "r"))==NULL)
        DieWithError("answers fopen() failed");
    if((user_info_ptr = fopen("dir_TES/user_info.txt", "a"))==NULL)
        DieWithError("user_info fopen() failed");

    long file_size, block_size, bytes_sent, bytes_left;
    char* ptr;
    for(;;){
        listen(sock_fd, 5);

        client_addr_len = sizeof(client_addr);
        new_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        read_buffer = tcpread_nbytes(new_fd, 4);
        printf("Received %sFrom %s:%d\n", read_buffer, inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        command = strtok(read_buffer, " ");
        printf("Command: %s\n", command);

        /*RQS*/
        if (strcmp(command, "RQS") == 0){
            char* user_answer;
            char answers[4];
            int i = 0;
            long QID;
            int SID;

            while(fscanf(user_info_ptr, "%ld %d %s", user_array[i].QID, user_array[i].SID, read_buffer)==3){
                strcpy(user_array[i].time_limit, read_buffer);
                i++;
                printf("read: %ld %d %s \n", user_array[i].QID, user_array[i].SID, read_buffer);
            }




            fscanf(answers_ptr, "%s %s %s %s %s", answers[0],answers[1],answers[2],answers[3],answers[4]);
            for(i=0; i<5; i++){
                printf("Answer to question %d: %s\n", i, user_answer);
                command = strtok(NULL, " ");
                if(strcmp(command, answers[i])!=0)
                    printf("Wrong answer on %d\n", i);
                else printf("Right answer on %d\n", i);
            }
        }
        /* RQT */
        else if(strcmp(command, "RQT")==0){
            long QID, SID;
            char time_limit[19];

            /*GET SID*/
            read_buffer = tcpread_until_char(new_fd, "\n", 6, 1);
            SID = atoi(read_buffer);
            printf("DEBUG: SID is: %d\n", SID);
            fprintf(user_info_ptr, "%d ", SID);

            /*GENERATE QID*/
            srand (time(NULL));
            QID = rand() % 900000 + 100000;
            printf("DEBUG: QID is: %ld\n", QID);
            fprintf(user_info_ptr, "%ld ", QID);

            /*GET Current Time*/
            strcpy(time_limit,get_time(600));
            printf("DEBUG: Time limit is: %ld\n", QID);
            fprintf(user_info_ptr, "%s\n", QID);

            /*Transmission Stuff*/
            fseek(file_ptr, 0, SEEK_END);
            file_size = ftell(file_ptr);
            rewind(file_ptr);

            strcpy(write_buffer, "AQT ");
            strcat(write_buffer, QID);
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

            while(bytes_left > 0){
                block_size = fread(write_buffer, sizeof(char), 256, file_ptr);
                bytes_left -= block_size;
                while(block_size > 0){
                    bytes_sent = write(new_fd, ptr, block_size);
                    if(write < 0){
                        printf("Error sending file");
                        break;
                    }
                    block_size -= bytes_sent;
                    ptr += bytes_sent;
                }
                ptr = &write_buffer[0];
                printf("Sending file... %d bytes sent\n", file_size-bytes_left);
            }
        }
        else{
            printf("Can't Recognize Transmission");
        }
    }
    fclose(file_ptr);
    fclose(answers_ptr);
    fclose(user_info_ptr);
    close(sock_fd);
    close(new_fd);
}
