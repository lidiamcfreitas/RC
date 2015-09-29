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
    unsigned short topic_port;
    char* ecp_name;
    char read_buffer[32];
    char write_buffer[256];
    char buffer[32];
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
        DieWithError("fopen() failed"); 
     
    long file_size, block_size, bytes_sent, bytes_left;
    char* ptr;
    for(;;){
        listen(sock_fd, 5); 
        client_addr_len = sizeof(client_addr);
        new_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        read(new_fd, read_buffer, sizeof(read_buffer));  
        printf("Received %sFrom %s:%d\n", read_buffer, inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        /*TODO - verificar conteudo do RQT <iiiii>*/
        
        fseek(file_ptr, 0, SEEK_END);
        file_size = ftell(file_ptr);
        rewind(file_ptr);
        
        /* TODO - ir buscar o numero do request e a hora */

        strcpy(write_buffer, "AQT 12345 09JAN2015_20:00:00 ");
        sprintf(buffer, "%d", file_size);
        strcat(write_buffer, buffer);
        strcat(write_buffer, " ");
        size_t message_size = strlen(write_buffer)*sizeof(char); 
        printf("Sending %s , size %d, file_size: %d\n", write_buffer, message_size,file_size);
        bytes_left = message_size;
        ptr = &write_buffer[0];        
        while(bytes_left > 0 ){
            bytes_sent = write(new_fd, ptr, bytes_left);
            bytes_left -= bytes_sent;
            ptr += bytes_sent;
        }
        /*Copiar código, mandar write_buffer pelo socket*/
        bytes_left = file_size;
        memset(write_buffer, '\0', 256);
        ptr = &write_buffer[0];
        
        while(bytes_left > 0){
            block_size = fread(write_buffer, sizeof(char), 256, file_ptr);
            bytes_left -= block_size;
            while(block_size > 0){
                bytes_sent = write(new_fd, ptr, block_size);
                block_size -= bytes_sent;
                ptr += bytes_sent;
            }
            ptr = &write_buffer[0];
            printf("Sending file... %d bytes sent\n", file_size-bytes_left); 
        }
    }
    fclose(file_ptr);
    close(sock_fd);
    close(new_fd);
}

