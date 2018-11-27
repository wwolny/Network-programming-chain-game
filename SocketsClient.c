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

   //Calling the error
   void error(char *msg) {
      perror(msg);
      exit(0);
   }

   //Freeing all the dynimacly allocated memory
   //If You want to allocate somewhere memory free it here
   void exitSmoothly() {}

   int main(int argc, char *argv[])
   {
     int sockfd = 0, newsockfd = 0, portno = 0, n = 0, i = 0;
     struct sockaddr_in serv_addr;
     struct hostent *server;

     //Buffer
     char buffer[BUFFSIZE];

     //Set of key messages for the program:
     //newWord() - holds the key with which every password is sent
     //If the message start with this sign
     //it means the server wants to get word from the player
     char newWord[] = "#";

     char stop[] = "Stop\n";

     //Just to wrap the connection connected code
     //If You wrap it is easier to move between
     //the begining of the program where we allocate memory
     //and create variables and the game loop
     if(TRUE) {
     if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
     }
     portno = atoi(argv[2]);
     server = gethostbyname(argv[1]);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) {
       error("ERROR opening socket");
     }

     if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
     serv_addr.sin_port = htons(portno);


     if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
      }
    }

    //Gameloop
    while (TRUE) {
      bzero(buffer, BUFFSIZE);

      if(read(sockfd, buffer, BUFFSIZE-1) < 0)  {
        error("ERROR read");
      }
      //When it gets the message to stop it get out of the loop
      if(strncmp(stop, buffer, sizeof(stop)) == 0) {
        break;
      }
      //If the first char is the keysign
       else if(buffer[0] == newWord[0]) {
        printf("You have to think of a word starting on the first letter of this word: %s\n", buffer);
        printf("Please give a word:\n");
        bzero(buffer, BUFFSIZE);
        scanf("%s", buffer);
	      send(sockfd, buffer, BUFFSIZE-1, 0);
        printf("\n");
      }
      else {
        printf("%s\n", buffer);
      }
    }
    return 0;
   }
