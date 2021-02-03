#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    int sockfd, port, wr;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    port = 1000; // hardcode port #
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // create tcp socket
    if (sockfd < 0) {
        perror("error:: unable to create socket");
        exit(1);
    }
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"error:: no such host\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("error:: unable to connect");
        exit(1);
    }
    printf("Send some text: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    wr = write(sockfd, buffer, strlen(buffer));
    if (wr < 0)
         perror("error:: writing to socket");
    bzero(buffer,256);
    wr = read(sockfd, buffer, 255);
    if (wr < 0)
         perror("error:: reading from socket");
    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}
