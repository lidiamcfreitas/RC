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
    char topic_name[26];
    char topic_ip[16];
    unsigned short topic_port;
    char to_send[2600];
    
    struct TES{
        char QName[255];
        char TESIp[16];
        unsigned short TESPort;  
    };

    struct TES TES_servers[99];
    
    
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
    
    if((file_ptr = fopen("dir_ECP/topics.txt", "r"))==NULL)
        DieWithError("fopen() failed");
    
    int i;
    i = 0;
    while(fscanf(file_ptr, "%s %s:%hu", topic_name, topic_ip, &topic_port)!=EOF){
        strcpy(TES_servers[i].QName, topic_name);
        strcpy(TES_servers[i].TESIp,  topic_ip);
        TES_servers[i].TESPort = topic_port;
        i++;
        printf("%s %s:%d", topic_name, topic_ip, topic_port);
    }
    fclose(file_ptr);    

    clntAddrLen = sizeof(clntAddr);
    
    for(;;){ 
        if((recvStringLen = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &clntAddr, &clntAddrLen))<0)
            DieWithError("recvfrom() failed");
        if (recvStringLen == strlen("TQR\n")){ /*when an TQR is received */
        }
        printf("Received %sFrom %s:%d\n", buffer, inet_ntoa(clntAddr.sin_addr),ntohs(clntAddr.sin_port));

    }
    close(sock_fd);
}
