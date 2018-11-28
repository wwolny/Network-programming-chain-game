#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE          1
#define FALSE         0
#define PORT          3000
#define TIMEOUT       1
#define BUFFSIZE      1024
#define MAX_CLIENTS   2
#define LIVING        1
#define DEAD          -1



//frees all the memory allocated during life of the programme
//if any new dynamic allocation done, add its freeing here
int exitSmoothly(fd_set *readfds, struct timeval *t, struct sockaddr_in *address) {
  free(readfds);
  free(address);
  free(t);
}

//If word given is correct, then return 1
//If its wrong return -1
int checkWord(char *prvWord, char *nxtWord) {
  if(prvWord[strlen(prvWord)-1] == nxtWord[0]) {
    return 1;
  } else {
    return -1;
  }
}

//Save the world given in a file
//If success return 1
//Otherwise return -1
int saveToFile(char *buffer, char* NameFile) {
    FILE* f = NULL;
    f = fopen(NameFile, "a");

    if (f != NULL) {
        fprintf(f,"%s\r\n",buffer);
        fclose(f);
        return 1;
    } else {
        printf("fail to open file\n");
        return -1;
        //exit(-1);
    }
}

//Read word from the file
//If success return 1
//Otherwise return -1
int ReadWord(char *Namefile,char *buffer, int lineNumber) {
    FILE *f=NULL;
    char s[255];
    int i=0;

    f = fopen(Namefile,"r");
    if(f == NULL) {
        printf("fail to open file\n");
        return -1;
        //exit(-1);
    }

    for (i = 0; i < lineNumber; i++) {
        fgets(s, sizeof(s), f);
        bzero(buffer,BUFFSIZE);
        strcat(buffer,s);
    }
    fclose(f);
    return 1;
}


//TODO: change the array of int of clients to this struct
struct player {
  int socketfd;
  short playing;
};

int main(int argc , char *argv[]) {
    int opt = TRUE;
    int master_socket = 0, addrlen = 0, new_socket = 0,
      activity = 0, i = 0, valread = 0, sd = 0;
    int max_sd = 0;
    int Nbrplayer = 0;//number of players in the game
    int curPlayer = 0;//number of current playing player
    int myFlag = FALSE; //Multiple use flag
    int finishGame = FALSE;
    int NbrWord = 0;
    struct player *client_socket1[MAX_CLIENTS];

    for(i = 0; i < MAX_CLIENTS; i++) {
      client_socket1[i] = (struct player*)malloc(sizeof(struct player));
      client_socket1[i]->socketfd = 0;
      client_socket1[i]->playing = LIVING;
    }

    struct sockaddr_in *address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    struct timeval *t = (struct timeval*)malloc(sizeof(struct timeval));

    char buffer[BUFFSIZE];  //data buffer of 1K

    char curWord[BUFFSIZE];//Buffer for current word

    //set of socket descriptors
    fd_set *readfds = NULL;
    readfds = (fd_set*)malloc(sizeof(fd_set));

    //Set of messages to control the flow of the programme
    //If You want to add new message implement it like this:
    //char newword[] = "text\n";
    char message[] = "Welcome to the word chain game ! \n ";

    char newClient[] = "New player joined the game \n";

    char stop[] = "Stop\n";

    char beginAll[] = "Let's begin the game\n";

    char firstLetter[] = "#a";

    char nextGot[] = "Next player got keyword\n";

    char mistake[] = "You have given wrong word\n";

    char winner[] = "You have won";

    char WordChain[] = "\nHere there is the Word chain of the game :\n";

    char NameFile[] = "File.txt";

    // /set first letter
    char c;
    srand(time(NULL));
    c = rand()%(26)+98;
    firstLetter[1] = c;
    printf("%s\n", firstLetter);

    //If You have a program to minimilize some part of code
    //You can wrap it here
    //it will be easier to move inside the main function
    //Inside is all network tools like setting the connection
    //TODO:Put as much as possible to external functions
    if(1) {
    //Initialise Timer for select()
    t->tv_sec=TIMEOUT;
    t->tv_usec=0;


    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket failed");
        exitSmoothly(readfds, t, address);
        for(i = 0; i < MAX_CLIENTS; i++) {
          free(client_socket1[i]);
        }
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections
    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exitSmoothly(readfds, t, address);
        for(i = 0; i < MAX_CLIENTS; i++) {
          free(client_socket1[i]);
        }
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons( PORT );

    //bind the socket to localhost port 3000
    if (bind(master_socket, (struct sockaddr*)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exitSmoothly(readfds, t, address);
        for(i = 0; i < MAX_CLIENTS; i++) {
          free(client_socket1[i]);
        }
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exitSmoothly(readfds, t, address);
        for(i = 0; i < MAX_CLIENTS; i++) {
          free(client_socket1[i]);
        }
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    }

  //Program waits until every player join the game
   while(Nbrplayer < MAX_CLIENTS) {
       	 //clear the socket set
       	 FD_ZERO(readfds);

      	  //add master socket to set
       	 FD_SET(master_socket, readfds);
       	 max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < MAX_CLIENTS ; i++) {
            //socket descriptor
            sd = client_socket1[i]->socketfd;
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET(sd, readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , readfds , NULL, NULL , t);

        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, readfds)) {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exitSmoothly(readfds, t, address);
                for(i = 0; i < MAX_CLIENTS; i++) {
                  free(client_socket1[i]);
                }
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(address->sin_addr) , ntohs (address->sin_port));
            Nbrplayer++;

            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) ) {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                //if position is empty
                if(client_socket1[i]->socketfd == 0 ) {
                    client_socket1[i]->socketfd = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
                send(client_socket1[i]->socketfd, newClient, strlen(newClient), 0);
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket1[i]->socketfd;
            if (FD_ISSET(sd, readfds)) {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, BUFFSIZE-1)) == 0) {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)address ,(socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address->sin_addr) , ntohs(address->sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket1[i]->socketfd = 0;
                }

                //Echo back the message that came in
                else {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
         }
	}

  printf("\n\n");
//Information about begining of the game send to every player
 for(i = 0; i < MAX_CLIENTS; i++) {
   if(client_socket1[i]->socketfd > 0) {
     if(send(client_socket1[i]->socketfd, beginAll, strlen(beginAll), 0) < 0) {
        perror("send");
     } else {
       printf("Send to %d\n" , i);
     }
   }
 }

 printf("\n\n");

  strcpy(curWord, firstLetter);
  //Game loop
  while(!finishGame) {
    //Sending a message that next player is making word
    //Sending to playing message with the word
    for(i = 0; i < MAX_CLIENTS; i++) {
      if(i != curPlayer || client_socket1[curPlayer]->playing == DEAD) {
        if(client_socket1[i]->socketfd > 0) {
           if(send(client_socket1[i]->socketfd, nextGot, strlen(nextGot), 0) < 0) {
              perror("send");
           } else {
             printf("Next player know that first player  id: %d\n" , i);
           }
        }
      } else {
        if(client_socket1[i]->socketfd > 0) {
          if(send(client_socket1[i]->socketfd, curWord, strlen(curWord), 0) < 0) {
             perror("send");
          } else {
            printf("Send to %d\n" , i);
          }
        }
      }
    }

    printf("\n\n");

    //Wait until the player will send the respond with word
    myFlag = 1;
    while (myFlag && client_socket1[curPlayer]->playing == LIVING) {
      FD_ZERO(readfds);
      FD_SET(master_socket, readfds);
      sd = client_socket1[curPlayer]->socketfd;
      if(sd > 0)
         FD_SET(sd, readfds);
      bzero(buffer,BUFFSIZE);
      if (FD_ISSET(sd, readfds)) {
        read(sd , buffer, BUFFSIZE-1);
        if(checkWord(curWord, buffer) < 0) {
          Nbrplayer--;
          client_socket1[curPlayer]->playing = DEAD;
          if(Nbrplayer == 1) {
            finishGame = TRUE;
          }
          if(send(client_socket1[curPlayer]->socketfd, mistake,
                  strlen(mistake), 0) < 0) {
             perror("Error to send");
          } else {
            printf("Player %d made mistake\n" , curPlayer);
          }
          break;
        }
        printf("Good answer for the player %d !\n\n", curPlayer);
        saveToFile(buffer, NameFile);
        NbrWord++;
        bzero(curWord, BUFFSIZE-1);
        strcpy(curWord, "#");
        strcat(curWord, buffer);
        printf("%s\n", curWord);
        myFlag = 0;
      }
    }
    curPlayer= (curPlayer+1)%MAX_CLIENTS;
  }

//Find the winner
  for (i = 0; i < MAX_CLIENTS; i++) {
    if(client_socket1[i]->playing == LIVING) {
      Nbrplayer = i;
      break;
    }
  }

//Winner and printing the chain
//Now NbrPlayer is the id of the winner
  bzero(curWord, BUFFSIZE-1);
  sprintf(curWord, "\nPlayer %d won!!!\n", Nbrplayer);
  for (i = 0; i < MAX_CLIENTS; i++) {
    if(i == Nbrplayer) {
      if(client_socket1[i]->socketfd > 0) {
         if(send(client_socket1[i]->socketfd, winner, strlen(winner), 0) < 0) {
            perror("send");
         } else {
           printf("Player %d won\n" , i);
         }
      }
    } else {
      if(client_socket1[i]->socketfd > 0) {
        if(send(client_socket1[i]->socketfd, curWord, strlen(curWord), 0) < 0) {
           perror("send");
        } else {
          printf("Player %d know who won\n" , i);
        }
      }
    }
  }

  //Word chain to send to the clients
  //Not Working yet
  // printf("%s\n",WordChain);
  // for (i=0; i <= NbrWord; i++) {
  //     bzero(buffer,BUFFSIZE-1);
  //     ReadWord(NameFile,buffer,i);
  //     printf("%s\n",buffer);
  //     printf(" ");
  // }
  printf("\n\n");

//Quiting all clients
  for(i = 0; i < MAX_CLIENTS; i++) {
    if(client_socket1[i]->socketfd > 0) {
      if(send(client_socket1[i]->socketfd, stop, strlen(stop), 0) < 0) {
          perror("send");
      }
      printf("Send to %d\n" , i);
    }
  }

  exitSmoothly(readfds, t, address);
  for(i = 0; i < MAX_CLIENTS; i++) {
    free(client_socket1[i]);
  }
  return 0;
}
