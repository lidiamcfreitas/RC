//
//  User.c
//
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#include "../CommonHeader.h"

void process_command(struct sockaddr_in ecpAddr, int udpsock_fd, int tcpsock_fd);

int main(int argc, char *argv[]){

    struct hostent *ecphostptr;
    struct sockaddr_in ecpAddr;
    unsigned short ecpPort;
    struct hostent *teshostptr;
    struct sockaddr_in tesAddr;
    unsigned short tesPort;
    int udpsock_fd;
    int tcpsock_fd;
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

    /* create the tcp socket */
    if((tcpsock_fd = socket(AF_INET, SOCK_STREAM, 0))<0)
      DieWithError("TCP socket() failed");

    /* define server address structure */
    memset(&ecpAddr, '\0', sizeof(ecpAddr));
    ecpAddr.sin_family = AF_INET;
    /* get ip string and then get the address */
    ecpAddr.sin_addr.s_addr = ((struct in_addr*)(ecphostptr->h_addr_list[0]))->s_addr;
    ecpAddr.sin_port = htons(ecpPort);


    for(;;){
        process_command(ecpAddr, udpsock_fd, tcpsock_fd);
    }
    close(udpsock_fd);
}

void process_command( struct sockaddr_in ecpAddr, int udpsock_fd, int tcpsock_fd)
{
    char command[8];
    char *topic_name;
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
        int num_topics;
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
            sscanf(topic_name,"T%d",&num_topics);

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





            /* -------------->>>>> FIX-ME <<<<<------------ needs to act on AWTES response */

    }

    /* SUBMIT */
    else if(strcmp(command, "submit")==0){
        char q1[2], q2[2], q3[2], q4[2], q5[2];

        scanf("%s %s %s %s %s", q1, q2, q3, q4, q5);


        /* -------------->>>>> FIX-ME <<<<<------------ needs to act on AWTES response */

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
