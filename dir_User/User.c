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
            printf("here");
            hostptr = gethostbyname(argv[2]);
        }
        else if(strcmp(argv[1], "-p") == 0)
        {
            servPort = atoi(argv[2]);
        }
     }

    /* create the UDP socket */
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
    	DieWithError("socket() failed");
    
    /* define server address structure */
    memset(&servAddr, '\0', sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    /* get ip string and then get the address */
    servAddr.sin_addr.s_addr = inet_addr(hostptr->h_addr_list[0]);
    servAddr.sin_port = htons(servPort);
    for(;;){
        process_command(servAddr, sock_fd);
    }
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
        char buffer[4];
        strcpy(buffer, "TQR");
        
        if(sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr*) &servAddr, sizeof(servAddr))<0)
            DieWithError("sendto() failed");
        
    } 
    /* REQUEST  */
    else if(strcmp(command, "request")==0){
        printf("request\n");
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

