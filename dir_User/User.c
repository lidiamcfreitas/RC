//
//  User.c
//  
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#include "../CommonHeader.h"

void process_command(struct sockaddr_in servAddr, int sock_fd);

int main(int argc, char *argv[]){

    struct hostent *hostptr;
    struct sockaddr_in servAddr;
    unsigned short servPort;
    int sock_fd;
    int broadcast;
    int ret;  
 
    broadcast = 1;


    if( argc < 1 || argc > 5 || argc % 2 != 1 ) /* test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s [-n ECPname] [-p ECPport]\n", argv[0]);
        exit(1);
    }
    
    /* default values */
    if((hostptr = gethostbyname("localhost"))<0){
        DieWithError("gethostbyname() failed");
    }
    servPort = DEFAULT_PORT;
     
    if(argc==5)
    {   /* considering that the order is correct */
        if((hostptr = gethostbyname(argv[2]))<0)
            DieWithError("gethostbyname() failed");
        servPort = atoi(argv[4]);
    }
    if(argc==3)
     {
        if(strcmp(argv[1], "-n") == 0) 
        {
            hostptr = gethostbyname(argv[2]);
        }
        else if(strcmp(argv[1], "-p") == 0)
        {
            servPort = atoi(argv[2]);
        }
     }

    /* create the UDP socket */
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    	DieWithError("socket() failed");
    

    /* define server address structure */
    memset(&servAddr, '\0', sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    /* get ip string and then get the address */
    servAddr.sin_addr.s_addr = ((struct in_addr*)(hostptr->h_addr_list[0]))->s_addr;
    servAddr.sin_port = htons(servPort);
    for(;;){
        process_command(servAddr, sock_fd);
    }
    close(sock_fd);
}

void process_command( struct sockaddr_in servAddr, int sock_fd)
{
    char command[8];
    memset(&command, '\0', sizeof(command));
    
    printf("> ");
    scanf("%s", command);
    
    if (strcmp(command, "exit")==0){
        exit(0);
    } 
    /* LIST */
    else if(strcmp(command, "list")==0){
        char send_buffer[4];
        char rcv_buffer[2476];
        int msg_size = 0;
        socklen_t addr_size;
        
        strcpy(send_buffer, "TQR\n");
        printf("Trying to list...\n");
    	if(sendto(sock_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*) &servAddr, sizeof(servAddr))<0)
       		DieWithError("sendto() failed");
        
        addr_size = sizeof(servAddr);
        printf("Message sent...\n");
	
        if(((msg_size = recvfrom(sock_fd, rcv_buffer, 2476, 0, (struct sockaddr*) &servAddr, &addr_size))<0))  
            DieWithError("recv() failed");
        rcv_buffer[msg_size] = '\0';
        printf("%s", rcv_buffer);
    } 
    /* REQUEST  */
    else if(strcmp(command, "request")==0){
        int request_no;
        char send_buffer[6];
        char req_buffer[3];
        
        scanf("%s", req_buffer);
        request_no = atoi(req_buffer);
        printf("request of topic %d\n", request_no);
        strcpy(send_buffer, "TER ");
        strcat(send_buffer, req_buffer);
        strcat(send_buffer, "\n");
        printf("%d \n" , send_buffer[5]);
    	
        if(sendto(sock_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*) &servAddr, sizeof(servAddr))<0)
       	    DieWithError("sendto() failed");
        
          
    }
    /* SUBMIT */ 
    else if(strcmp(command, "submit")==0){
        char q1[2], q2[2], q3[2], q4[2], q5[2];
        
        scanf("%s %s %s %s %s", q1, q2, q3, q4, q5);
    } 
    /* HELP */
    else if(strcmp(command, "help")==0){
        printf("available commands:\n"
                    "\tlist\n"
                    "\trequest\n"
                    "\tsubmit X X X X X\n"
                    "\texit\n"
                    "\thelp\n");
    } else { /* NOT FOUND */
        printf("%s: command not found. write 'help' for available commands.\n", command);
    }
}

