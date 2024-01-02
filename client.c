#include <stdio.h>
#include "unp.h"
#include <sys/socket.h>
#include <arpa/inet.h>



void cli(FILE *fp, int sockfd){
    char sendline[MAXLINE], recvline[MAXLINE];
    // find the ip address of the client
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    getsockname(sockfd, (SA *) &cliaddr, &len);
    printf("Client IP address: %s\n", inet_ntoa(cliaddr.sin_addr));
    // printf("Client port: %d\n", ntohs(cliaddr.sin_port));
    // send the ip address and port to the server
    sprintf(sendline, "%s", inet_ntoa(cliaddr.sin_addr));
    Write(sockfd, sendline, strlen(sendline));
    // receive the ack from the server
    if(read(sockfd, recvline, MAXLINE) == 0){
        printf("str_cli: server terminated prematurely\n");
        return;
    }else{
        printf("recv: %s\n", recvline);
    }
}

int main(int argc, char **argv){

    int sockfd;
    struct sockaddr_in servaddr;
    if(argc != 2){
        printf("usage: ./client <IPaddress>\n");
        return 0;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_port = htons(SERV_PORT); // daytime server
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    cli(stdin, sockfd); // do it all

    return 0;
}