#include "../CommonHeader.h"

int main(int argc, char *argv[]){
    struct hostent *hostptr;
    struct sockaddr_in server_addr;
    unsigned short server_port;
    int sock_fd;
    int broadcast;
    int ret;  
    char write_buffer[32];
    char read_buffer[256];
    broadcast = 1;
    memset(read_buffer, '\0', 256);

    if( argc < 1 || argc > 5 || argc % 2 != 1 ) /* test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s [-n ECPname] [-p ECPport]\n", argv[0]);
        exit(1);
    }
    
    /* default values */
    if((hostptr = gethostbyname("localhost"))<0){
        DieWithError("gethostbyname() failed");
    }
    server_port = DEFAULT_PORT;
     
    if(argc==5)
    {   /* considering that the order is correct */
        if((hostptr = gethostbyname(argv[2]))<0)
            DieWithError("gethostbyname() failed");
        server_port = atoi(argv[4]);
    }
    if(argc==3)
     {
        if(strcmp(argv[1], "-n") == 0) 
        {
            hostptr = gethostbyname(argv[2]);
        }
        else if(strcmp(argv[1], "-p") == 0)
        {
            server_port = atoi(argv[2]);
        }
     }

    /* create the UDP socket */
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0))<0)
    	DieWithError("socket() failed");
    

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ((struct in_addr*)(hostptr->h_addr_list[0]))->s_addr;
    server_addr.sin_port = htons(server_port);
    
    connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));    

    int bytes_written, bytes_read, bytes = 10, bytes_left = 10;
    strcpy(write_buffer, "RQT ");
    strcat(write_buffer, "12345\n");

    char* ptr;
    ptr = &write_buffer[0];
     
    while(bytes_left > 0){
        bytes_written = write(sock_fd, ptr, bytes_left);
        bytes_left -= bytes_written;
        ptr += bytes_written; 
    }
    ptr = &read_buffer[0];
    int done = 0;
    bytes_left = 29;
    printf("reading...\n");
    while( bytes_left > 0 ){
        bytes_read = read(sock_fd, ptr, bytes_left);
        bytes_left -= bytes_read;
        ptr += bytes_read; 
        printf("read %d\n, bytes_read");
    }
    printf("parsing...\n");
    char* data;
    data = strtok(read_buffer, " ");
    while(data != NULL){
        printf("%s\n", data);
        data = strtok(NULL," ");
    }
    ptr = &read_buffer[0];
    bytes_read = read(sock_fd, ptr, 1);
    
    char c = ptr[0];
    printf("read... %c\n", c);
    while( c != ' '){
        ptr += bytes_read;
        bytes_read = read(sock_fd, ptr, 1);
        c = ptr[0];
        printf("read ... %c \n" , c);
    }
    ptr[0] = '\0';
    long total_bytes = 0, file_size = atoi(read_buffer);
    bytes_left = file_size;
    char* test_buffer = malloc(sizeof(char)*256+2);
    ptr = &test_buffer[0];
    FILE* end_file = fopen("new_file.pdf", "w");
    while(bytes_left > 0){
        bytes_read = read(sock_fd, ptr, 256);
        bytes_left -= bytes_read;
        ptr += bytes_read;
        total_bytes += bytes_read;
        ptr[0] = '\0';
        bytes_written = fwrite(test_buffer, sizeof(char), bytes_read, end_file);
        printf("Downloaded %d written %d \n" , bytes_read, bytes_written);
        free(test_buffer);
        test_buffer = malloc(sizeof(char)*256);
        ptr = &test_buffer[0];
     }
    fclose(end_file);
    printf("Received file with size: %s \n", read_buffer);  
}
