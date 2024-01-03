#include <stdio.h>
#include "unp.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

void clear_recv_send(char *recvline, char *sendline){
    bzero(recvline, MAXLINE);
    bzero(sendline, MAXLINE);
}

void cli(FILE *fp, int sockfd){
    int n, mode = -1;
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
    



    stage1: printf("===========================stage 1===============================\n"); // stage 1: choose login or register
    clear_recv_send(recvline, sendline);
    if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
    else{
        recvline[n] = '\0';
        printf("recv: %s\n", recvline);
    }
   
    scanf("%d", &mode); // change the enter string to int


    if(mode == 1){
        sprintf(sendline, "%s", "1");
        printf("Send: %s\n", sendline);
        Write(sockfd, sendline, strlen(sendline));
    }
    else if(mode == 2){
       
        sprintf(sendline, "%s", "2");
        printf("Send: %s\n", sendline);
        Write(sockfd, sendline, strlen(sendline));
    }
    else{
        printf("Invalid input\n");
        goto stage1;
    }

    stage2: printf("===========================stage 2===============================\n"); // stage 2: enter id and password
    clear_recv_send(recvline, sendline);
    if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
    else{
        recvline[n] = '\0';
        printf("recv: %s\n", recvline);
        if(mode == 1){
            // login in
            char id[100], password[100];
            scanf("%s %s", id, password);
            sprintf(sendline, "%s %s", id, password);
            printf("Send: %s\n", sendline);
            Write(sockfd, sendline, strlen(sendline));
            if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
            else{
                recvline[n] = '\0';
                printf("recv: %s\n", recvline);
                if(strcmp(recvline, "Login successfully!") == 0){
                    // goto stage3;
                }
                else{
                    printf("Login failed\n");
                    goto stage1;
                }
            }
            

        }
        else{
            // register
            char id[100], password[100];
            scanf("%s %s", id, password);
            sprintf(sendline, "%s %s", id, password);
            printf("Send: %s\n", sendline);
            Write(sockfd, sendline, strlen(sendline));
            if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
            else{
                recvline[n] = '\0';
                printf("recv: %s\n", recvline);
                if(strcmp(recvline, "Register successfully! Login automatically.") == 0){
                    // goto stage3;
                }
                else{
                    printf("Register failed\n");
                    goto stage2;
                }
            }
        }
    }
    stage3: printf("===========================stage 3===============================\n"); // stage 3: choose the function
    clear_recv_send(recvline, sendline);
    if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
    else{
        recvline[n] = '\0';
        printf("recv: %s\n", recvline);
    }
    scanf("%d", &mode); // change the enter string to int
    if(mode == 1){
        sprintf(sendline, "%s", "1");
        printf("Send: %s", sendline);
        Write(sockfd, sendline, strlen(sendline));
        if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
        else{
            recvline[n] = '\0';
            printf("recv: %s\n", recvline);
            if(strcmp(recvline,"Cannot find an online opponent.\nPlease try again later or create an empty room\n") == 0){
                goto stage3;
            }
            else{
                printf("Enter the game\n");
                // todo: enter the game
            }
        }
    }
    else if(mode == 2){
        sprintf(sendline, "%s", "2");
        printf("Send: %s\n", sendline);
        Write(sockfd, sendline, strlen(sendline));
        if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
        else{
            recvline[n] = '\0';
            printf("recv: %s\n", recvline);
            if(strcmp(recvline,"Cannot create a room.\nPlease try again later") == 0){
                goto stage3;
            }
            else{
                printf("wait for another player\n");
                // todo: enter the game
            }
        }
    }
    else if(mode == 3){
        sprintf(sendline, "%s", "3");
        printf("Send: %s", sendline);
        Write(sockfd, sendline, strlen(sendline));
    }
    else if(mode == 4){
        sprintf(sendline, "%s", "4");
        printf("Send: %s", sendline);
        Write(sockfd, sendline, strlen(sendline));
    }
    else if(mode == 5){
        sprintf(sendline, "%s", "5");
        printf("Send: %s", sendline);
        Write(sockfd, sendline, strlen(sendline));
    }
    else{
        printf("Invalid input\n");
        goto stage3;
    }
    terminate_prematurely: printf("Server terminated prematurely\n");
        return;
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