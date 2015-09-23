//
//  ECP.c
//  
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#include "../CommonHeader.h"

int main(int argc, char *argv[]){
    
    int sock_fd;
    struct sockaddr_in servAddr, clntAddr;
    unsigned int clntAddrLen;
    unsigned short servPort;
    char buffer[255]; /* FIX: WHAT SIZE? */
    int recvStringLen;
    FILE *file_ptr;

    struct TES{
        char QName[255];
        int TESIp;
        unsigned short TESPort;  
    };
    
    
    if(argc==1){
        servPort = DEFAULT_PORT;
    } else if(argc == 3 && (strcmp(argv[1], "-p") == 0)){
        servPort = atoi(argv[2]);
    } else {
    	fprintf(stderr, "Usage: %s [-p ECPname]\n", argv[0]);
    	exit(1);
    }
   
    if((sock_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
       DieWithError("socket() failed");
    
    /* define server address structure */
    memset(&servAddr, '\0', sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    if(bind(sock_fd, (struct sockaddr*) &servAddr, sizeof(servAddr))<0)
    	DieWithError("bind() failed");
    
    clntAddrLen = sizeof(clntAddr);
    
    for(;;){ 
        if((recvStringLen = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &clntAddr, &clntAddrLen))<0)
            DieWithError("recvfrom() failed");
        if (recvStringLen == strlen("TQR\n")){ /*when an TQR is received */
            if((file_ptr = fopen("dir_ECP/topics.txt", "r"))==NULL)
            	DieWithError("fopen() failed");
            printf("received");
            while(fscanf(file_ptr, "%s %s:%d",awtes, ip, port,)!=EOF){
                	
            }
            fclose(file_ptr);    
        }
        printf("Received %sFrom %s:%d\n", buffer, inet_ntoa(clntAddr.sin_addr),ntohs(clntAddr.sin_port));

    }
    close(sock_fd);
}
