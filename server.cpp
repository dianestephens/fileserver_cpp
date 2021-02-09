#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

void * connection_thread(void* p_client_socket);

int main(int argc, char *argv[])
{
     int sockfd, clientsocket, port;
     socklen_t clen;

     struct sockaddr_in serv_addr, clientInfo;

     // create a tcp (SOCK_STREAM) socket
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        perror("error:: Could not open socket.");
        exit(1);
      }
     // clear address structure
     bzero((char *) &serv_addr, sizeof(serv_addr));

     port = 1000; // hard code port #
     // setup the host_addr structure
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port); // network byte order

     // bind socket to port
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
              perror("error:: unable to bind");
     // listen for incoming connections - backlog queue set to 5.
     listen(sockfd,50);

     while (true) { // loop accepting connections
        clen = sizeof(clientInfo);
        printf("Ready for connections ...");
        // accept connection from client socket and put the client socket info in clientInfo
        clientsocket  = accept(sockfd,(struct sockaddr *) &clientInfo, &clen);
        if (clientsocket < 0)
          perror("error:: on accept");

        printf("Connected to %s on port %d\n",
            inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));

        pthread_t t;
        int *pclient = (int *)malloc(sizeof(int));
        *pclient = clientsocket;
        pthread_create(&t, NULL, connection_thread, pclient);
      } // while
      return 0;
}

void * connection_thread(void* p_client_socket) {
  int clientsocket = *((int*)p_client_socket);
  free(p_client_socket);
  char buffer[256];
  int n;

  bzero(buffer,256);

  n = read(clientsocket,buffer,255);
  if (n < 0)
    perror("ERROR reading from socket");
  printf("You sent: %s\n",buffer);
  send(clientsocket, buffer, n, 0);
  close(clientsocket);
  printf("closing connection\n");
  return NULL;
}
