//
//
//  GRUPO 26 
//  Jorge Santos, Lidia Freitas, Miguel Vera
//
//

#include "../CommonHeader.h"
#include <time.h>       /* time */


void process_command(struct sockaddr_in ecpAddr, int udpsock_fd, int sid);
int tcpinit(char * tes_addr, short unsigned tes_port);
 
int request_done;
struct sockaddr_in server_addr;
char tes_addr[16];
unsigned short tes_port;
char* QID;

int main(int argc, char *argv[]){

    struct hostent *ecphostptr;
    struct sockaddr_in ecpAddr;
    unsigned short ecpPort;
    int udpsock_fd;
    int broadcast;
    int sid;
    char* sid_str;
    broadcast = 1;
    request_done = 0;

    if( argc < 2 || argc > 6 || argc % 2 != 0 ) /* test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s [SID] [-n ECPname] [-p ECPport]\n", argv[0]);
        exit(1);
    }

    /* default values */
    if((ecphostptr = gethostbyname("localhost"))<0){
        DieWithError("gethostbyname() failed");
    }
    ecpPort = DEFAULT_PORT;

    if(argc==6)
    {   /* considering that the order is correct */
        if((ecphostptr = gethostbyname(argv[3]))<0)
            DieWithError("gethostbyname() failed");
        ecpPort = atoi(argv[5]);
        sid_str = argv[1];
    }
    if(argc==4)
    {
        if(strcmp(argv[2], "-n") == 0)
        {
            ecphostptr = gethostbyname(argv[2]);
        }
        else if(strcmp(argv[2], "-p") == 0)
        {
            ecpPort = atoi(argv[2]);
        }
        sid_str = argv[1];
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


    /* get SID */
    //srand (time(NULL));
    //sid = rand() % 90000 + 10000;
    sid = atoi(sid_str);
    if(sid < 10000 || sid > 99999){
        printf("Wrong format for student number provided.\n");
        exit(1);
    }
    printf("You are now identified as %d.\n", sid);
    
    for(;;){
        process_command(ecpAddr, udpsock_fd, sid);
    }
    close(udpsock_fd);
}

void process_command( struct sockaddr_in ecpAddr, int udpsock_fd, int sid)
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

        /* Send TQR\n to ECP */
        strcpy(send_buffer, "TQR\n");
        if(sendto(udpsock_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*) &ecpAddr, sizeof(ecpAddr))<0)

            DieWithError("sendto() failed");

        addr_size = sizeof(ecpAddr);
        printf("(Debug)TQR sent...\n");

        /* RECEIVE TOPICS */
        if(((msg_size = recvfrom(udpsock_fd, rcv_buffer, 2476, 0, (struct sockaddr*) &ecpAddr, &addr_size))<0))
            DieWithError("recv() failed");
        rcv_buffer[msg_size] = '\0';
        printf("(DEBUG)Received: %s \n", rcv_buffer);

        if(strcmp("EOF\n", rcv_buffer)==0)
            DieWithError("No topics to show.\n");

        else if(strcmp("ERR\n", rcv_buffer)==0)
            DieWithError("Error reading TQR\n");

        else{ /*parse AWT nX Topic1 Topic2 ... */
            topic_name = strtok(rcv_buffer, " ");

            if(strcmp(topic_name,"AWT")!=0)
                DieWithError("Could not recognize AWT\n");

            /* print topics list */
            topic_name = strtok(NULL, " ");
            num_topics = atoi(topic_name);
            printf("(Debug) Received %d topics.\n", num_topics);
            for(i = 1; i<=num_topics;i++){
                topic_name = strtok(NULL, " ");
                printf("%d. %s\n", i, topic_name);
            }
        }
    }
    /* REQUEST  TOPIC*/
    else if(strcmp(command, "request")==0){
        request_done = 1; 
        char request_no[3];
        char send_buffer[16];
        char rcv_buffer[32];
        char buffer[32];
        int msg_size = 0;
        socklen_t addr_size;

        /* sending */
        scanf("%s", request_no);
        printf("Sending request of topic %s\n", request_no);
        strcpy(send_buffer, "TER ");
        strcat(send_buffer, request_no);
        strcat(send_buffer, "\n");
        printf("%s \n" , send_buffer);

        if(sendto(udpsock_fd, send_buffer, strlen(send_buffer), 0, (struct sockaddr*) &ecpAddr, sizeof(ecpAddr))<0)
            DieWithError("sendto() failed");
        addr_size = sizeof(ecpAddr);
        /* receiving */
        if(((msg_size = recvfrom(udpsock_fd, rcv_buffer, sizeof(rcv_buffer), 0, (struct sockaddr*) &ecpAddr, &addr_size))<0))
            DieWithError("recv() failed");

        rcv_buffer[msg_size] = '\0';
        printf("(DEBUG)Received: %s\n", rcv_buffer);

        if(strcmp("EOF\n", rcv_buffer)==0)
            DieWithError("No topics to show.\n");
        else if(strcmp("ERR\n", rcv_buffer)==0)
            DieWithError("Error reading TQR\n");
        else if(msg_size > 28){
            printf("(DEBUG) msg_size: %d", msg_size);
            DieWithError("Received message too large");
        }
        else{ /*parse AWTES IPTES PORTES ... */
            printf("(Debug)parsing AWTES\n");
            topic_name = strtok(rcv_buffer, " ");

            if(strcmp(topic_name,"AWTES")!=0)
                printf("Could not recognize AWTES\n");
            topic_name = strtok(NULL, " ");
            strcpy(tes_addr, topic_name);
            topic_name = strtok(NULL, " ");
            tes_port = atoi(topic_name);

            tcpsock_fd = tcpinit(tes_addr, tes_port);

            /*Preparing RQT packet*/
            strcpy(send_buffer, "RQT ");
            sprintf(buffer, "%d", sid);
            strcat(send_buffer, buffer);
            strcat(send_buffer, "\n");
            
            printf("(Debug)Sending request to %d\n", tcpsock_fd);
            tcpwrite(tcpsock_fd, send_buffer, 10);
            
            char* data = tcpread_nbytes(tcpsock_fd, 4);
            printf("(Debug)Received response: %s\n", data);
            data = tcpread_until_char(tcpsock_fd, ' ', 26, 1);
            
            /*Saving QID for later operations*/
            printf("(Debug)QID: %s\n", data);
            QID = malloc(sizeof(char)*strlen(data));
            strcpy(QID, data);
            
            /*Time limit*/    
            data = tcpread_nbytes(tcpsock_fd, 19);
            printf("(Debug)Time limit: %s\n", data);
            printf("Your deadline is %s\n", data);
            
            /*File transfer - */
            data = tcpread_until_char(tcpsock_fd, ' ', 32, 1);
            long total_bytes = 0, file_size = atoi(data);
            int bytes_left, bytes_read, bytes_written;
            char* test_buffer = malloc(sizeof(char)*256+2);
            FILE* end_file = fopen("questionnaire.pdf", "w");
            char* ptr;
       
            bytes_left = file_size;
            ptr = &test_buffer[0];
            
            printf("Starting file download. %lu bytes left\n", bytes_left);
            while(bytes_left > 0){
                bytes_read = read(tcpsock_fd, ptr, 256);
                bytes_left -= bytes_read;
                ptr += bytes_read;
                total_bytes += bytes_read;
                ptr[0] = '\0';
                bytes_written = fwrite(test_buffer, sizeof(char), bytes_read, end_file);
                free(test_buffer);
                test_buffer = malloc(sizeof(char)*256);
                ptr = &test_buffer[0];
                //if(bytes_read != 0)
                  //  printf("read %d , wrote %d, left %d\n", bytes_read, bytes_written, bytes_left);
            }
            printf("Downloaded file with success!\n");
            fclose(end_file);
        }
    }

    /* SUBMIT */
    else if(strcmp(command, "submit")==0){
        tcpsock_fd = tcpinit(tes_addr, tes_port);
        if(request_done == 1 && tcpsock_fd != 0 ){
            char q1[2], q2[2], q3[2], q4[2], q5[2];
            char send_buffer[46];
            char SID[6];
            int stringLen;
            
            sprintf(SID, "%d", sid); 
            
            /*Readin user answer*/
            scanf("%s %s %s %s %s", q1, q2, q3, q4, q5);

            /*Preparing RQS*/
            strcpy(send_buffer,"RQS ");
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
            strcat(send_buffer, "\n");
            stringLen = strlen(send_buffer);
            printf("DEBUG: Sending: %s\n", send_buffer);
            
            tcpwrite(tcpsock_fd, send_buffer, stringLen);
	        char* response = tcpread_nbytes(tcpsock_fd,4);
            if(strcmp(response, "AQS ")!=0){
                printf("(DEBUG)Wrong response from server ( got %s )\n", response);
                DieWithError("Unexpected response");
	        }else{

                printf("(DEBUG)Received AQS\n");
                response = tcpread_until_char(tcpsock_fd, ' ',24, 1);
                printf("(DEBUG)QID: %s\n", response);
                response = tcpread_until_char(tcpsock_fd, '\n',5, 0);
                /*confirming message structure*/
                int j = 0, found = 0;
                for( j = 0; j < 5; j++){
                    if(response[j] == '\n')
                        found = 1;
                }
                if(found == 0)
                    DieWithError("Received incorrect message structure from server");
                printf("You scored %s percent on your questionnaire!",response);

           }
		/* -------------->>>>> FIX-ME <<<<<------------*/


            close(tcpsock_fd);
	    }
        else
        {
            printf("Can't submit before requesting. Request a questionnaire first.\n");
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

/*Helper function to init tcp connection each time*/
int tcpinit(char* tes_addr, short unsigned tes_port){
    int tcpsock_fd;
    struct sockaddr_in tcp_addr;

    if((tcpsock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
        DieWithError("socket() failed");
    /* construct server address structure */
    memset(&tcp_addr, '\0', sizeof(tcp_addr));
    server_addr.sin_family = AF_INET;
    printf("(Debug)Starting TCP connection on: %s %d\n", tes_addr, tes_port);
    server_addr.sin_addr.s_addr = inet_addr(tes_addr);
    server_addr.sin_port = htons(tes_port);
    if( connect(tcpsock_fd, (struct sockaddr*)&server_addr, sizeof(tcp_addr)) < 0)
        DieWithError("connect() failed");
    return tcpsock_fd;
}
