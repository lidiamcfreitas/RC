//
//  User.c
//
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#include "../CommonHeader.h"

void process_command(struct sockaddr_in ecpAddr, int udpsock_fd);
int tcpinit(char * tesPort, char* tesAddr, struct sockaddr_in tcpAddr);

int main(int argc, char *argv[]){

    struct hostent *ecphostptr;
    struct sockaddr_in ecpAddr;
    unsigned short ecpPort;
    int udpsock_fd;
    int broadcast;
    int ret;

    broadcast = 1;


    if( argc < 1 || argc > 5 || argc % 2 != 1 ) /* test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s [-n ECPname] [-p ECPport]\n", argv[0]);
        exit(1);
    }

    /* default values */
    if((ecphostptr = gethostbyname("localhost"))<0){
        DieWithError("gethostbyname() failed");
    }
    ecpPort = DEFAULT_PORT;

    if(argc==5)
    {   /* considering that the order is correct */
        if((ecphostptr = gethostbyname(argv[2]))<0)
            DieWithError("gethostbyname() failed");
        ecpPort = atoi(argv[4]);
    }
    if(argc==3)
     {
        if(strcmp(argv[1], "-n") == 0)
        {
            ecphostptr = gethostbyname(argv[2]);
        }
        else if(strcmp(argv[1], "-p") == 0)
        {
            ecpPort = atoi(argv[2]);
        }
     }

    /* create the UDP socket */
    if ((udpsock_fd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    	DieWithError("UDP socket() failed");


    /* define server address structure */
    memset(&ecpAddr, '\0', sizeof(ecpAddr));
    ecpAddr.sin_family = AF_INET;
    /* get ip string and then get the address */
    ecpAddr.sin_addr.s_addr = ((struct in_addr*)(ecphostptr->h_addr_list[0]))->s_addr;
    ecpAddr.sin_port = htons(ecpPort);


    for(;;){
        process_command(ecpAddr, udpsock_fd);
    }
    close(udpsock_fd);
}

void process_command( struct sockaddr_in ecpAddr, int udpsock_fd)
{
    char command[8];
    char *topic_name;
    int tcpsock_fd = 0;
    struct sockaddr_in tcpAddr;

    memset(&command, '\0', sizeof(command));

    printf("> ");
    scanf("%s", command);
    printf("\n");

    if (strcmp(command, "exit")==0){
        exit(0);
    }
    /* LIST */
    else if(strcmp(command, "list")==0){
        char send_buffer[5];
        char rcv_buffer[2476];
        int msg_size = 0;
        int num_topics, i;
        socklen_t addr_size;

        strcpy(send_buffer, "TQR\n");
        printf("Asking for list of topics...\n");
    	if(sendto(udpsock_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*) &ecpAddr, sizeof(ecpAddr))<0)
       		DieWithError("sendto() failed");

        addr_size = sizeof(ecpAddr);
        printf("Message sent...\n");

        /* RECEIVE TOPICS */
        if(((msg_size = recvfrom(udpsock_fd, rcv_buffer, 2476, 0, (struct sockaddr*) &ecpAddr, &addr_size))<0))
            DieWithError("recv() failed");
        rcv_buffer[msg_size] = '\0';
        printf("(DEBUG)Received: %s \n", rcv_buffer);

        if(strcmp("EOF\n", rcv_buffer)==0)
            printf("No topics to show.\n");

        else if(strcmp("ERR\n", rcv_buffer)==0)
            printf("Error reading TQR\n");

        else{ /*parse AWT nX Topic1 Topic2 ... */
            topic_name = strtok(rcv_buffer, " ");

            if(strcmp(topic_name,"AWT")!=0)
              printf("Could not recognize AWT\n");

            topic_name = strtok(NULL, " ");
            num_topics = atoi(topic_name);
            for(int i = 1; i<=num_topics;i++){
                topic_name = strtok(NULL, " ");
                printf("%d. %s\n", i, topic_name);
            }
        }
    }
    /* REQUEST  */
    else if(strcmp(command, "request")==0){
      char request_no[3];
      char send_buffer[10];
      char rcv_buffer[30];
      int msg_size = 0;
      char tesAddr[16];
      unsigned short tesPort;
      socklen_t addr_size;

      /* sending */
      scanf("%s", request_no);
      printf("request of topic %s\n", request_no);
      strcpy(send_buffer, "TER ");
      strcat(send_buffer, request_no);
      strcat(send_buffer, "\n");
      printf("%s \n" , send_buffer);

      if(sendto(udpsock_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*) &ecpAddr, sizeof(ecpAddr))<0)
           DieWithError("sendto() failed");

      /* receiving */
      if(((msg_size = recvfrom(udpsock_fd, rcv_buffer, sizeof(rcv_buffer), 0, (struct sockaddr*) &ecpAddr, &addr_size))<0))
          DieWithError("recv() failed");

      rcv_buffer[msg_size] = '\0';
      printf("(DEBUG)Received: %s", rcv_buffer);

      if(strcmp("EOF\n", rcv_buffer)==0)
          printf("No topics to show.\n");
      else if(strcmp("ERR\n", rcv_buffer)==0)
          printf("Error reading TQR\n");
      else if(msg_size != 28){
          printf("(DEBUG) msg_size: %d", msg_size);
      }


      else{ /*parse AWTES IPTES PORTES ... */
          topic_name = strtok(rcv_buffer, " ");

          if(strcmp(topic_name,"AWTES")!=0)
            printf("Could not recognize AWTES\n");
          topic_name = strtok(NULL, " ");
          strcpy(tesAddr, topic_name);
          topic_name = strtok(NULL, " ");
          tesPort = atoi(topic_name);
      }

      tcpsock_fd = tcpinit(tesAddr, tesPort, tcpAddr);




            /* -------------->>>>> FIX-ME <<<<<------------ needs to act on AWTES response */

    }

    /* SUBMIT */
    else if(strcmp(command, "submit")==0){
        if(tcpsock_fd != 0 ){
          char q1[2], q2[2], q3[2], q4[2], q5[2];
          char send_buffer[46];
          char QID[24] = "111222333444555666777";
          char SID[6] = "78980";
          int stringLen;

          scanf("%s %s %s %s %s", q1, q2, q3, q4, q5);

          if(connect(tcpsock_fd, (struct sockaddr*) &tcpAddr, sizeof(tcpAddr))<0)
                    DieWithError("connect() failed");


          strcat(send_buffer,"RQS ");
          strcat(send_buffer,SID);
          strcat(send_buffer, " ");
          strcat(send_buffer,QID);
          strcat(send_buffer, " ");
          strcat(send_buffer,q1);
          strcat(send_buffer, " ");
          strcat(send_buffer,q2);
          strcat(send_buffer, " ");
          strcat(send_buffer,q3);
          strcat(send_buffer, " ");
          strcat(send_buffer,q4);
          strcat(send_buffer, " ");
          strcat(send_buffer,q5);
          stringLen = strlen(send_buffer);

          printf("%s", send_buffer);
          if(send(tcpsock_fd, send_buffer, stringLen, 0)!= stringLen)
                    DieWithError("send() sent a different number of bytes than expected");

        /* -------------->>>>> FIX-ME <<<<<------------*/
      }
      else{

        printf("Can't submit before requesting\n");
      }

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
    printf("\n");
}


int tcpinit(char* tesPort, char* tesAddr, struct sockaddr_in tcpAddr){

    int tcpsock_fd;

    tesPort = atoi(tesPort);

    if((tcpsock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
      DieWithError("socket() failed");


    /* construct server address structure */
      memset(&tcpAddr, 0, sizeof(tcpAddr));
      tcpAddr.sin_family = AF_INET;
      tcpAddr.sin_addr.s_addr = inet_addr(tesAddr);
      tcpAddr.sin_port = htons(tesPort);

      return tcpsock_fd;

}
