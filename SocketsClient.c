   #include <stdio.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netdb.h>
   #include <string.h>   //strlen
   #include <stdlib.h>
   #include <errno.h>
   #include <unistd.h>   //close
   #include <arpa/inet.h>
   #include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

   #define TRUE   1
   #define FALSE  0
   #define BUFFSIZE 1024

   void error(char *msg)
   {
       perror(msg);
       exit(0);
   }

   int main(int argc, char *argv[])
   {
     int sockfd, newsockfd, portno, n, i;
     struct sockaddr_in serv_addr;
     struct hostent *server;
     char buffer[BUFFSIZE];

     char stop[] = "Stop\n";

     if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
     }
     portno = atoi(argv[2]);
     server = gethostbyname(argv[1]);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0)
         error("ERROR opening socket");


     if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
         exit(0);
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
     serv_addr.sin_port = htons(portno);


    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
         error("ERROR connecting");

    while (TRUE) {
      bzero(buffer, BUFFSIZE);

      if(read(sockfd, buffer, BUFFSIZE-1) < 0) {
        error("ERROR read");
      }
      printf("%s", buffer);

      if(strncmp(stop, buffer, sizeof(stop)) == 0) {
        break;
      }
    }

     return 0;
   }
