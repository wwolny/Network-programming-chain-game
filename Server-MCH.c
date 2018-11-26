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

#define TRUE   1
#define FALSE  0
#define PORT 3000
#define TIMEOUT 1
#define BUFFSIZE 1024

//frees all the memory allocated during life of the programme
//if any new dynamic allocation done, add its freeing here
int exitSmoothly(fd_set *readfds, fd_set *writefds, struct timeval *t, struct sockaddr_in *address) {
  free(readfds);
  free(writefds);
  free(address);
  free(t);
}

//If word given is correct, then return 1
//If its wrong return -1
int checkWord(char *prvWord, char *nxtWord) {
  if(prvWord[strlen(prvWord)] == nxtWord[0]) {
    return 1;
  } else {
    return -1;
  }
}

struct player {
  int socketfd;
  short playing;
}

int main(int argc , char *argv[]) {
    int opt = TRUE;
    int master_socket = 0, addrlen = 0, new_socket = 0, client_socket[5],
          max_clients = 5, activity = 0, i = 0, valread = 0, sd = 0;
    int max_sd = 0;
    int Nbrplayer = 0;
    int curPlayer = 0;
    int myFlag = 0;


    struct sockaddr_in *address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    struct timeval *t = (struct timeval*)malloc(sizeof(struct timeval));

    char buffer[BUFFSIZE];  //data buffer of 1K

    char curWord[BUFFSIZE];

    //set of socket descriptors
    fd_set *readfds = NULL;
    readfds = (fd_set*)malloc(sizeof(fd_set));
    fd_set *writefds = NULL;
    writefds = (fd_set*)malloc(sizeof(fd_set));

    //a message
    char message[] = "Welcome to the word chain game ! \n ";

    char newClient[] = "New player joined the game \n";

    char stop[] = "Stop\n";

    char beginAll[] = "Let's begin the game\n";

    char firstLetter[] = "#a\n";

    char nextGot[] = "Next player got keyword\n";


    if(1) {

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    //Initialise Timer for select()
    t->tv_sec=TIMEOUT;
    t->tv_usec=0;


    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket failed");
        exitSmoothly(readfds, writefds, t, address);
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exitSmoothly(readfds, writefds, t, address);
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons( PORT );

    //bind the socket to localhost port 3000
    if (bind(master_socket, (struct sockaddr*)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exitSmoothly(readfds, writefds, t, address);
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exitSmoothly(readfds, writefds, t, address);
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    }

   while(Nbrplayer < max_clients) {
       	 //clear the socket set
       	 FD_ZERO(readfds);

      	  //add master socket to set
       	 FD_SET(master_socket, readfds);
       	 max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) {
            //socket descriptor
            sd = client_socket[i];

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
                exitSmoothly(readfds, writefds, t, address);
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
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
                send(client_socket[i], newClient, strlen(newClient), 0);
            }

        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, BUFFSIZE-1)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)address ,(socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address->sin_addr) , ntohs(address->sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
         }
	}

//Information about begining of the game
 for(i = 0; i < max_sd; i++) {
   if(client_socket[i] > 0) {
     if(send(client_socket[i], beginAll, strlen(beginAll), 0) < 0) {
        perror("send");
     } else {
       printf("Send to %d\n" , i);
     }
   }
 }

//Choose first letter and send it to the first player
  for(i = 0; i < max_sd; i++) {
    if(myFlag) {
      if(client_socket[i] > 0) {
         if(send(client_socket[i], nextGot, strlen(nextGot), 0) < 0) {
            perror("send");
         } else {
           printf("Next player know that first player  id: %d\n" , i);
         }
      }
    } else {
      if(client_socket[i] > 0) {
        if(send(client_socket[i], firstLetter, strlen(firstLetter), 0) < 0) {
           perror("send");
        } else {
          printf("Send to %d\n" , i);
        }
        myFlag = 1;
      }
    }
  }
//First listning for first word
  myFlag = 1;
  while (myFlag) {
    //clear the socket set
    FD_ZERO(readfds);

    //add master socket to set
    FD_SET(master_socket, readfds);
    max_sd = master_socket;

    //add child sockets to set
    for ( i = 0 ; i < max_clients ; i++) {
        //socket descriptor
        sd = client_socket[i];

        //if valid socket descriptor then add to read list
        if(sd > 0)
            FD_SET(sd, readfds);

        //highest file descriptor number, need it for the select function
        if(sd > max_sd)
            max_sd = sd;
    }
    //Reading
    for (i = 0; i < max_clients; i++) {
        sd = client_socket[i];
        bzero(buffer,BUFFSIZE);

        if (FD_ISSET(sd, readfds)) {
            //read the incoming message
            read(sd , buffer, BUFFSIZE-1);
            printf("%s\n", buffer);
            bzero(curWord, BUFFSIZE-1);
            strcpy(curWord, "#");
            strcat(curWord, buffer);
            printf("%s\n", curWord);
            myFlag = 0;
            break;
        }
    }
  }
  curPlayer= (curPlayer+1)%max_clients;

  //Game loop
  while(1) {
      for(i = 0; i < max_sd; i++) {
        if(i != curPlayer) {
          if(client_socket[i] > 0) {
             if(send(client_socket[i], nextGot, strlen(nextGot), 0) < 0) {
                perror("send");
             } else {
               printf("Next player know that first player  id: %d\n" , i);
             }
          }
        } else {
          if(client_socket[i] > 0) {
            if(send(client_socket[i], curWord, strlen(curWord), 0) < 0) {
               perror("send");
            } else {
              printf("Send to %d\n" , i);
            }
          }
        }
      }

      myFlag = 1;
      while (myFlag) {
        FD_ZERO(readfds);
        FD_SET(master_socket, readfds);
        sd = client_socket[curPlayer];
        if(sd > 0)
           FD_SET(sd, readfds);
        bzero(buffer,BUFFSIZE);
        if (FD_ISSET(sd, readfds)) {
          read(sd , buffer, BUFFSIZE-1);
          bzero(curWord, BUFFSIZE-1);
          strcpy(curWord, "#");
          strcat(curWord, buffer);
          printf("%s\n", curWord);

          myFlag = 0;
          break;
        }
      }
      curPlayer= (curPlayer+1)%max_clients;
  }

//Quiting all clients
  for(i = 0; i < max_sd; i++) {
    if(client_socket[i] > 0)
    {
      if(send(client_socket[i], stop, strlen(stop), 0) < 0)
      {
          perror("send");
      }

      printf("Send to %d\n" , i);
    }
  }

  exitSmoothly(readfds, writefds, t, address);
  return 0;
}
