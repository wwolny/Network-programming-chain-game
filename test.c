#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#define MAX_CLIENTS   5
#define LIVING        1

struct player {
  int socketfd;
  short playing;
};

int main(int argc , char *argv[]) {
  int i = 0;
  struct player *client_socket1[MAX_CLIENTS];

  int sd1;
  for(i = 0; i < MAX_CLIENTS; i++) {
    client_socket1[i] = (struct player*)malloc(sizeof(struct player));
    client_socket1[i]->socketfd = i;
    client_socket1[i]->playing = LIVING;
  }

  printf("client_socket1[3]->socketfd: %d\n", client_socket1[3]->socketfd);
  //printf("*client_socket1[3]->socketfd: %d\n", *client_socket1)[3]->socketfd;
  //printf("%s\n", client_socket1[3]);

  for(i = 0; i < MAX_CLIENTS; i++) {
    free(client_socket1[i]);
  }
}
