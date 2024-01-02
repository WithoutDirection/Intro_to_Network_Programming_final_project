
#include <pthread.h>
#include <semaphore.h>
#include "unp.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>




#define max_client_num 10

int is_used[max_client_num] = {0};
char ip[max_client_num][100];
int  connfd[max_client_num];
char sendline[max_client_num][MAXLINE], recvline[max_client_num][MAXLINE];
pthread_t thread[max_client_num];
sem_t is_used_sem, connfd_sem;
void sig_chld(int signo){
    pid_t pid;
    int stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
        ;
    return;
}

void clear_recv_send(int i){
    bzero(recvline[i], MAXLINE);
    bzero(sendline[i], MAXLINE);
}

int login(char *account, char *password){
    DIR *dir;
    struct dirent *ptr;
    char path[100] = "./user/";
    dir = opendir(path);
    while((ptr = readdir(dir)) != NULL){
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)continue;
        if(strcmp(ptr->d_name, account) == 0){
            // find the account
            char account_path[100];
            sprintf(account_path, "%s%s", path, account);
            FILE *fp = fopen(account_path, "r");
            char correct_password[100];
            fscanf(fp, "%s", correct_password);
            if(strcmp(correct_password, password) == 0){
                // correct password
                fclose(fp);
                return 1;
            }
            else{
                // wrong password
                fclose(fp);
                return 0;
            }
        }
    }
    // cannot find the account
    return 0;
}

void register_account(char *account, char *password){
    char path[100] = "./user/";
    char account_path[100];
    sprintf(account_path, "%s%s", path, account);
    FILE *fp = fopen(account_path, "w");
    fprintf(fp, "%s", password);
    fclose(fp);
}

void *funct(int *arg ){
    int i = *arg, n, mode;
    clear_recv_send(i);
    printf("thread %d is created\n", i);

    stage1: printf("======================================================\n"); // stage 1: decide login or register

    sprintf(sendline[i], "Connect successfully!\nPress 1 to login\nPress 2 to register a new account and then login\n");
    Write(connfd[i], sendline[i], strlen(sendline[i]));
    printf("send to %s: %s\n", ip[i], sendline[i]);
    if(n = Read(connfd[i], recvline[i], MAXLINE) == 0){
        printf("client %d terminated prematurely\n", i);
        sem_wait(&is_used_sem);
        is_used[i] = 0;
        sem_post(&is_used_sem);
        sem_post(&connfd_sem);
        pthread_exit(NULL);
    }
    else{
        recvline[i][n] = '\0';
        printf("receive from %s: %s\n", ip[i], recvline[i]);
        // check if the received is a number
        int is_num = 1;
        for(int j = 0; j < strlen(recvline[i]); j++){
            if(recvline[i][j] == " ")continue;
            if(recvline[i][j] < '0' || recvline[i][j] > '9'){
                is_num = 0;
                break;
            }
        }
        if(is_num == 0){
            sprintf(sendline[i], "Invalid input. Please try again\n");
            Write(connfd[i], sendline[i], strlen(sendline[i]));
            goto stage1;
        }
        else{
            mode = atoi(recvline[i]);
            printf("mode = %d\n", mode);
        }
    }
    
    stage2: printf("======================================================\n"); // stage 2: login or register
    clear_recv_send(i);
    if(mode == 1){
        sprintf(sendline[i], "Please enter your account and password in format: <account> <password>\n");
        Write(connfd[i], sendline[i], strlen(sendline[i]));
        if(n = Read(connfd[i], recvline[i], MAXLINE) == 0){
            printf("client %d terminated prematurely\n", i);
            sem_wait(&is_used_sem);
            is_used[i] = 0;
            sem_post(&is_used_sem);
            sem_post(&connfd_sem);
            pthread_exit(NULL);
        }
        else{
            char account[100], password[100];
            sscanf(recvline[i], "%s %s", account, password);
            // find the account in the user folder
            if(login(account, password) == 1){
                // correct password
                sprintf(sendline[i], "Login successfully!\n");
                Write(connfd[i], sendline[i], strlen(sendline[i]));
                printf("send to %s: %s\n", ip[i], sendline[i]);
            }
            else{
                // wrong password
                sprintf(sendline[i], "Wrong password. Please try again\n");
                Write(connfd[i], sendline[i], strlen(sendline[i]));
                printf("send to %s: %s\n", ip[i], sendline[i]);
                goto stage2;
            }

        }
    }
    else if(mode == 2){
        sprintf(sendline[i], "Please enroll your account and password in format: <account> <password>\n");
        Write(connfd[i], sendline[i], strlen(sendline[i]));
        if(n = Read(connfd[i], recvline[i], MAXLINE) == 0){
            printf("client %d terminated prematurely\n", i);
            sem_wait(&is_used_sem);
            is_used[i] = 0;
            sem_post(&is_used_sem);
            sem_post(&connfd_sem);
            pthread_exit(NULL);
        }
        else{
            char account[100], password[100];
            sscanf(recvline[i], "%s %s", account, password);
            // enroll the account in the user folder
            register_account(account, password);
            sprintf(sendline[i], "Register successfully!\n");
            Write(connfd[i], sendline[i], strlen(sendline[i]));

        }
    }

    stage3: printf("======================================================\n"); // stage 3: choose the function

        
}
 

int main(){
    int listenfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    
    sem_init(&is_used_sem, 0, 1);
    sem_init(&connfd_sem, 0, max_client_num);
    
    
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);      /* must call waitpid() */

    for(;;){
        printf("wait for connfd_sem\n");
        sem_wait(&connfd_sem);
        printf("connfd_sem is released\n");
        clilen = sizeof(cliaddr);
        int tmp_connfd;
        if((tmp_connfd= accept(listenfd, (SA *) &cliaddr, &clilen)) < 0){
            if(errno == EINTR)
                continue;
            else
                err_sys("accept error");
        }
        else{
            printf("====================================\n");
            
            for(int i = 0; i < max_client_num; i++){
                if(is_used[i] == 0){
                    bzero(sendline[i], MAXLINE);
                    bzero(recvline[i], MAXLINE);
                    printf("find %d is not used\n", i);
                    printf("connfd[%d] = %d\n", i, tmp_connfd);
                    connfd[i] = tmp_connfd;
                    sem_wait(&is_used_sem);
                    is_used[i] = 1;
                    sem_post(&is_used_sem);
                    printf("client %d is connected\n", i);
                    Inet_ntop(AF_INET, &cliaddr.sin_addr, ip[i], sizeof(ip[i]));
                    Read(connfd[i], recvline[i], MAXLINE);
                    printf("receive from %s: %s\n", ip[i], recvline[i]);
                    pthread_create(&thread[i], NULL, (void *) &funct, (void *) i);
                    printf("====================================\n");
                    break;
                    
                }
                else{
                    if(i == max_client_num - 1) {
                        // printf("No more client can be connected\n");
                        i = 0;
                    }
                }
            }
            // int val = 0;
            // sem_getvalue(&connfd_sem, &val);
            // printf("connfd_sem = %d\n", val);
            // printf("is used:\n");
            // for(int i = 0; i < 10; i++){
            //     printf("%d is used, connfd: %d\n",i, connfd[i]);
            // }       
            printf("====================================\n");     
        }
        
    }


    return 0;
}