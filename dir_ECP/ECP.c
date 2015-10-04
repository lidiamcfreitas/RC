//
//  ECP.c
//  
//
//
//

#include "../CommonHeader.h"

int main(int argc, char *argv[]){
    
    int sock_fd;
    struct sockaddr_in servAddr, clnt_addr;
    unsigned int clnt_addr_len;
    unsigned short serv_port;
    char buffer[255]; /* FIX: WHAT SIZE? */
    int recv_string_len;
    FILE *file_ptr;
    char topic_name[26];
    char topic_ip[16];
    unsigned short topic_port;
    char to_send[2600];
    int i;
    
    struct TES{
        char QName[255];
        char TESIp[16];
        unsigned short TESPort;  
    }TES_servers[99];
    int num_TES = 0;

    if(argc==1){
        serv_port = DEFAULT_PORT;
    } else if(argc == 3 && (strcmp(argv[1], "-p") == 0)){
        serv_port = atoi(argv[2]);
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
    servAddr.sin_port = htons(serv_port);

    if(bind(sock_fd, (struct sockaddr*) &servAddr, sizeof(servAddr))<0)
    	DieWithError("bind() failed");
    
    if((file_ptr = fopen("dir_ECP/topics.txt", "r"))==NULL)
        DieWithError("fopen() failed");

    i = 0;
    while(fscanf(file_ptr, "%s %[^:]:%hu", topic_name, topic_ip, &topic_port)==3){
        strcpy(TES_servers[i].QName, topic_name);
        strcpy(TES_servers[i].TESIp,  topic_ip);
        TES_servers[i].TESPort = topic_port;
        i++;
        num_TES++;
        printf("read: %s %s %hu \n", topic_name, topic_ip, topic_port); 
    }
    fclose(file_ptr);    

    clnt_addr_len = sizeof(clnt_addr);
    
    for(;;){ 
        if((recv_string_len = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &clnt_addr, &clnt_addr_len))<0)
            DieWithError("recvfrom() failed");
        buffer[recv_string_len] = '\0';

        /* TQR */
        if (strcmp("TQR\n",buffer)==0 && (recv_string_len==strlen("TQR\n")) /*because of possible double \0*/)
        {
            strcpy(to_send, "AWT ");
            sprintf(topic_name, "%d ", num_TES); /* using topic_name just to reuse variable */
            strcat(to_send, topic_name);
            if (num_TES==0)
            { /* No topics available */
                if (sendto(sock_fd, "EOF\n", strlen("EOF\n"), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                    DieWithError("sendto() failed");
            } else {
                for(i=0; i<num_TES; i++)
                {
                    strcat(to_send, TES_servers[i].QName);
                    strcat(to_send, " ");
                    printf("%s\n", to_send);
                }
                to_send[strlen(to_send)-1] = '\n'; /* substitute ' ' for '\n'" */
                if (sendto(sock_fd, to_send, strlen(to_send), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                    DieWithError("sendto() failed");
            }
        }
        /* TER */
        else if(!strncmp(buffer, "TER ", 4) && (recv_string_len==strlen("TER 1\n") || recv_string_len==strlen("TER 99\n"))){
            sscanf(buffer, "TER %d", &i);
            i--; /* 1...99 to 0..98 */
            if(i<0 || i>99){
                if (sendto(sock_fd, "EOF\n", strlen("EOF\n"), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                    DieWithError("sendto() failed");
            } else {
                strcpy(to_send, "AWTES ");
                strcat(to_send, TES_servers[i].TESIp);
                strcat(to_send, " ");
                sprintf(topic_name, "%d", TES_servers[i].TESPort); /* using topic_name just to reuse variable */
                strcat(to_send, topic_name);
                strcat(to_send, "\n");

                if (sendto(sock_fd, to_send, strlen(to_send), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                    DieWithError("sendto() failed");
            }
        }
        /* IQR */
        else if(!strncmp(buffer, "IQR ", 4)){
            
            char iqr[4];
            char qid[255];
            int sid;
            char topname[26];
            int score;

            FILE *f = fopen("dir_ECP/stats.txt", "a");
            if (f == NULL)
                DieWithError("Error opening file: stats.txt");
            
             //memmove(buffer, buffer+4, strlen(buffer) - 4 + 1);

            if(sscanf(buffer,"%s %d %s %s %d",iqr, &sid, qid, topname, &score)!=5)
            {
                if (sendto(sock_fd, "ERR\n", strlen("ERR\n"), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                    DieWithError("sendto() failed");
            }
            fprintf(f, "%d %s %s %d", sid, qid, topname, score);

            fclose(f);

            strcpy(to_send, "AWI ");
            strcat(to_send, qid);

            if (sendto(sock_fd, to_send, strlen(to_send), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                DieWithError("sendto() failed");
        }
        /* else -> ERR*/
        else
        {
            if (sendto(sock_fd, "ERR\n", strlen("ERR\n"), 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr))<0)
                DieWithError("sendto() failed");
        }
        printf("Received %sFrom %s:%d\n", buffer, inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));

    }
    close(sock_fd);
}
