
#include <pthread.h>
#include <semaphore.h>
#include "unp.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <math.h>




#define max_client_num 10

int is_used[max_client_num] = {0};
char online_account[max_client_num][100];
char ip[max_client_num][100];
int  connfd[max_client_num];
char sendline[max_client_num][MAXLINE], recvline[max_client_num][MAXLINE];
pthread_t thread[max_client_num];

typedef struct room room_t;

struct room{
    int people_num;
    int room_id;
    char player1[100], player2[100];
    int player1_rank, player2_rank;     
};

room_t room_list[max_client_num/2];

void init_room_list(){
    for(int i = 0; i < max_client_num/2; i++){
        room_list[i].people_num = 0;
        room_list[i].room_id = i;
        room_list[i].player1_rank = 0;
        room_list[i].player2_rank = 0;
    }
}

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

int check_the_cmd(char *cmd){
    // check if the cmd is number
    int is_num = 1;
    for(int i = 0; i < strlen(cmd); i++){
        if(cmd[i] == ' ')continue;
        if(cmd[i] < '0' || cmd[i] > '9'){
            is_num = 0;
            break;
        }
    }
    if(is_num == 0){
        return -1;
    }
    else{
        // change the cmd string to int
        int num = 0;
        for(int i = 0; i < strlen(cmd); i++){
            if(cmd[i] == ' ')continue;
            num = num * 10 + cmd[i] - '0';
        }
        return num;
    }
}

int login(char *account, char *password){
    DIR *dir;
    struct dirent *ptr;
    char path[100] = "./user/";
    dir = opendir(path);
    while((ptr = readdir(dir)) != NULL){
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)continue;
        if(strcmp(ptr->d_name, account) == 0){
            char password_file_path[200];
            sprintf(password_file_path, "%s%s/%s", path, account, "password.txt");

            FILE *fp = fopen(password_file_path, "r");
            if(fp == NULL){
                printf("cannot open the file\n");
                // cannot open the file
                return 0;
            }

            char correct_password[100];
            fgets(correct_password, 100, fp);
            fclose(fp);
            if(strcmp(correct_password, password) == 0){
                // correct passwordm return 
                return 1;
            }
            else{
                // wrong password
                return 0;
            }

        }
    }
    // cannot find the account
    return 0;
}

int register_account(char *account, char *password){
    // printf("account: %s, password: %s", account, password);
    char path[100] = "./user/";
    char account_path[100];
    char password_file_path[200];
    // check if the account name has been used and the the folder exists
    DIR *dir;
    struct dirent *ptr;
    // printf("path = %s\n", path);
    dir = opendir(path);
    printf("find whether the account name: %s has been used\n", account);
    while((ptr = readdir(dir)) != NULL){
        // printf("ptr->d_name = %s\n", ptr->d_name);
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)continue;
        if(strcmp(ptr->d_name, account) == 0){
            // find the account
            return 0;
        }
    }
    // printf("the account name has not been used\n");
    // create the account folder and set the password as a txt file
    sprintf(account_path, "%s%s", path, account);
    mkdir(account_path, 0777);
    sprintf(password_file_path, "%s/%s", account_path, "password.txt");
    FILE *fp = fopen(password_file_path, "w");
    fprintf(fp, "%s", password);
    fclose(fp);
    return 1;
    
}

int find_an_onlone_opponent(int i){
    // find an online opponent
    int opponent = -1;
    // check the rank of the i-th player with the rank.txt in its account folder
    char path[100] = "./user/";
    char account_path[100], rank_path[100];
    sprintf(account_path, "%s%s", path, online_account[i]);
    sprintf(rank_path, "%s/rank.txt", account_path);
    FILE *fp = fopen(rank_path, "r");
    int rank;
    fscanf(fp, "%d", &rank);
    fclose(fp);
    // find the opponent with the min rank difference
    int min_rank_diff = 1000000;
    for(int j = 0; j < max_client_num/2; j++){
        if(room_list[j].people_num == 1){
            int tmp = room_list[j].player1_rank - rank;
            if(tmp < 0)tmp = -tmp;
            if(tmp < min_rank_diff){
                min_rank_diff = tmp;
                opponent = j;
            }
        }
    }
    // if cannot find an opponent, return -1
    if(opponent == -1){
        return -1;
    }

    printf("find opponent in room%d: %s\n", opponent ,room_list[opponent].player1);
    room_list[opponent].people_num = 2;
    strcpy(room_list[opponent].player2, online_account[i]);
    room_list[opponent].player2_rank = rank;

    return opponent;
    
}

int create_room(){
    // create an empty room
    int room_idx = -1;
    for(int i = 0; i < max_client_num/2; i++){
        if(room_list[i].people_num == 0){
            room_idx = i;
            break;
        }
    }
    if(room_idx == -1){
        return -1;
    }
    else{
        // create the a random room id
        srand(time(NULL));
        int room_id = rand() % 1000;
        room_id = pow(room_id, 2) - 1;
        room_list[room_idx].room_id = room_id;

        room_list[room_idx].people_num = 1;
        strcpy(room_list[room_idx].player1, online_account[room_idx]);
        // load the rank of the player
        char path[100] = "./user/";
        char account_path[100], rank_path[100];
        sprintf(account_path, "%s%s", path, online_account[room_idx]);
        sprintf(rank_path, "%s/rank.txt", account_path);
        FILE *fp = fopen(rank_path, "r");
        int rank;
        fscanf(fp, "%d", &rank);
        fclose(fp);
        room_list[room_idx].player1_rank = rank;
        return room_idx;
    }
}

int find_room_by_id(int room_id){
    for(int i = 0; i < max_client_num/2; i++){
        if(room_list[i].room_id == room_id){
            return i;
        }
    }
    return -1;
}

void *funct(int *arg ){
    int i = *arg, n, mode, return_value;
    clear_recv_send(i);
    printf("thread %d is created\n", i);

    stage1: printf("=======================stage 1===============================\n"); // stage 1: decide login or register

    sprintf(sendline[i], "Connect successfully!\nPress 1 to login\nPress 2 to register a new account and then login\n");
    Write(connfd[i], sendline[i], strlen(sendline[i]));
    printf("send to %s: %s\n", ip[i], sendline[i]);
    if((n = Read(connfd[i], recvline[i], MAXLINE)) == 0) goto terminate_prematurely;
    else{
        recvline[i][n] = '\0';
        printf("receive from %s: %s\n", ip[i], recvline[i]);
        // check if the received is a number
        mode = check_the_cmd(recvline[i]);
        printf("receive mode = %d\n", mode);
        if(mode == -1){
            printf("wrong command, please try again\n");
        }
    }
    
    stage2: printf("======================stage 2================================\n"); // stage 2: login or register
    clear_recv_send(i);
    char account[100], password[100];
    if(mode == 1){
        sprintf(sendline[i], "Please enter your account and password in format: <account> <password>\n");
        printf("send to %s: %s\n", ip[i], sendline[i]);
        Write(connfd[i], sendline[i], strlen(sendline[i]));
        if((n = Read(connfd[i], recvline[i], MAXLINE)) == 0) goto terminate_prematurely;
        else{
            sscanf(recvline[i], "%s %s", account, password);
            // find the account in the user folder
            if(login(account, password) == 1){
                // correct password
                sprintf(sendline[i], "Login successfully!");
                Write(connfd[i], sendline[i], strlen(sendline[i]));
                printf("send to %s: %s\n", ip[i], sendline[i]);
            }
            else{
                // wrong password
                sprintf(sendline[i], "Wrong password. Please try again\n");
                Write(connfd[i], sendline[i], strlen(sendline[i]));
                printf("send to %s: %s\n", ip[i], sendline[i]);
                goto stage1;
            }

        }
    }
    else if(mode == 2){
        sprintf(sendline[i], "Please enroll your account and password in format: <account> <password>\n");
        Write(connfd[i], sendline[i], strlen(sendline[i]));
        if((n = Read(connfd[i], recvline[i], MAXLINE)) == 0) goto terminate_prematurely;
        else{
            recvline[i][n] = '\0';
            sscanf(recvline[i], "%s %s", account, password);
            // printf("receive account: %s, password: %s\n", account, password);
            // enroll the account in the user folder
            if(register_account(account, password)){
                sprintf(sendline[i], "Register successfully! Login automatically.");
                Write(connfd[i], sendline[i], strlen(sendline[i]));
            }
            else{
                sprintf(sendline[i], "The account name has been used. Please try another one\n");
                Write(connfd[i], sendline[i], strlen(sendline[i]));
                goto stage2;
            }
           

        }
    }
    // add the account to the online account list
    strcpy(online_account[i], account);
    

    stage3: printf("=========================stage 3=================================\n"); // stage 3: choose the function

    clear_recv_send(i);
    sprintf(sendline[i], "Find an online opponent: press 1\nCreate an empty room: press2\nEnter a room with room id: press 3\nReview the history game: press 4\nPlay with AI: press 5\n");
    Write(connfd[i], sendline[i], strlen(sendline[i]));
    printf("send to %s: %s\n", ip[i], sendline[i]);
    if(n = Read(connfd[i], recvline[i], MAXLINE) == 0) goto terminate_prematurely;
    else{
        recvline[i][n] = '\0';
        printf("receive from %s: %s\n", ip[i], recvline[i]);
        // check if the received is a number
        mode = check_the_cmd(recvline[i]);
        if(mode == -1){
            printf("wrong command, please try again\n");
            goto stage3;
        }
    }

    clear_recv_send(i);

    if(mode == 1){
        return_value = find_an_onlone_opponent(i);
        if(return_value == -1){
            sprintf(sendline[i], "Cannot find an online opponent.\nPlease try again later or create an empty room\n");
            Write(connfd[i], sendline[i], strlen(sendline[i]));
            // printf("send to %s: %s\n", ip[i], sendline[i]);
            goto stage3;
        }
        else{
            sprintf(sendline[i], "Find an online opponent successfully!\nYour opponent is %s\nHis/Her rank is %d\n", room_list[return_value].player1, room_list[return_value].player1_rank);
            Write(connfd[i], sendline[i], strlen(sendline[i]));
            
        }
    }
    else if(mode == 2){
        return_value = create_room();
        if(return_value == -1){
            sprintf(sendline[i], "Cannot create a room.\nPlease try again later\n");
            Write(connfd[i], sendline[i], strlen(sendline[i]));
            // printf("send to %s: %s\n", ip[i], sendline[i]);
            goto stage3;
        }
        else{
            sprintf(sendline[i], "Create a room successfully!\nYour room id is %d\n", room_list[return_value].room_id);
            Write(connfd[i], sendline[i], strlen(sendline[i]));
        }
    }
    else if(mode == 3){
        int room_id;
        sprintf(sendline[i], "Please enter the room id\n");
        Write(connfd[i], sendline[i], strlen(sendline[i]));
        if(n = Read(connfd[i], recvline[i], MAXLINE) == 0) goto terminate_prematurely;
        else{
            recvline[i][n] = '\0';
            printf("receive from %s: %s\n", ip[i], recvline[i]);
            // check if the received is a number
            room_id = check_the_cmd(recvline[i]);
            if(room_id == -1){
                printf("wrong command, please try again\n");
                goto stage3;
            }
            else{
                return_value = find_room_by_id(room_id);
                if(return_value == -1){
                    sprintf(sendline[i], "Cannot find the room.\nPlease try again later\n");
                    Write(connfd[i], sendline[i], strlen(sendline[i]));
                    // printf("send to %s: %s\n", ip[i], sendline[i]);
                    goto stage3;
                }
                else{
                    sprintf(sendline[i], "Enter the room successfully!\nYour opponent is %s\nHis/Her rank is %d\n", room_list[return_value].player1, room_list[return_value].player1_rank);
                    Write(connfd[i], sendline[i], strlen(sendline[i]));
                    room_list[return_value].people_num = 2;
                    strcpy(room_list[return_value].player2, online_account[i]);
                    // load the rank of the player
                    char path[100] = "./user/";
                    char account_path[100], rank_path[100];
                    sprintf(account_path, "%s%s", path, online_account[i]);
                    sprintf(rank_path, "%s/rank.txt", account_path);
                    FILE *fp = fopen(rank_path, "r");
                    int rank;
                    fscanf(fp, "%d", &rank);
                    fclose(fp);
                    room_list[return_value].player2_rank = rank;
                    
                }
            
            }
        }
    }



    terminate_prematurely: printf("client %d terminated prematurely\n", i); // terminate prematurely
    sem_wait(&is_used_sem);
    is_used[i] = 0;
    sem_post(&is_used_sem);
    sem_post(&connfd_sem);
    printf("=========================================\n");
    pthread_exit(NULL);

        
}
 

int main(){
    init_room_list();
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
                    int *idx = (int *)malloc(sizeof(int));
                    *idx = i;
                    pthread_create(&thread[i], NULL, (void *) funct, idx);
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