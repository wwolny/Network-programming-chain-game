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

   void error(char *msg) {
      perror(msg);
      exit(0);
   }

   void exitSmoothly(struct sockaddr_in *serv_addr) {
      free (serv_addr);
   }

   int main(int argc, char *argv[])
   {
     int sockfd = 0, newsockfd = 0, portno = 0, n = 0, i = 0;
     struct sockaddr_in serv_addr; //= (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
     struct hostent *server;
     char buffer[BUFFSIZE];
     char newWord[] = "#";

     char stop[] = "Stop\n";

     if(TRUE) {
     if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        //exitSmoothly(serv_addr);
        exit(0);
     }
     portno = atoi(argv[2]);
     server = gethostbyname(argv[1]);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) {
       //exitSmoothly(serv_addr);
       error("ERROR opening socket");
    }

     if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        //exitSmoothly(serv_addr);
        exit(0);
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
     serv_addr.sin_port = htons(portno);


    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        //exitSmoothly(serv_addr);
        error("ERROR connecting");
    }

    while (TRUE) {
      bzero(buffer, BUFFSIZE);

      if(read(sockfd, buffer, BUFFSIZE-1) < 0)  {
        //exitSmoothly(serv_addr);
        error("ERROR read");
      }
      printf("%s", buffer);

      if(strncmp(stop, buffer, sizeof(stop)) == 0) {
        break;
      }
      if(buffer[0] == newWord[0]) {
        printf("You have to think of a word starting on the first letter of this word: %s\n", buffer);
        printf("Please give a word:\n");
        bzero(buffer, BUFFSIZE);
        scanf("%s", buffer);
	      send(sockfd, buffer, BUFFSIZE-1, 0);
      }
    }
    //exitSmoothly(serv_addr);
    return 0;
   }
