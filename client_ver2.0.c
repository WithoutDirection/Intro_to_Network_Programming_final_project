#include "unp.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// #include "game.h"
#define maxsize = 8;

#define BOUNDARY(x, y) ((x) >= 1 && (x) <= 8 && (y) >= 1 && (y) <= 8)

typedef struct board board_t;

struct board{
    int piece[64];
    int piece_last[64];
    int color;
    int sign;
};

void init_board(board_t bd){
    for(int i = 0; i < 64; i++){
        bd.piece[i] = 0;
        bd.piece_last[i] = 0;
    }
    bd.piece[27] = bd.piece[36] = -1;
    bd.piece[28] = bd.piece[35] = 1;
    bd.color = 1;
    bd.sign = -1;

}

int introduce()
{
    printf("        [遊戲規則]\n\n");
    printf("[開始]  開始時, 黑棋位於E4和D5, 白棋位於D4和E5, 黑方先\n\n");
    printf("[落子]  在空位下一枚棋子, 在該棋子需在直、橫、斜八個方向中至少有一枚己方棋子,\n");
    printf("        且兩棋子中間的敵方棋子會全部翻轉為我方棋子\n\n");
    printf("[結束]  當雙方都不能落子時, 遊戲結束, 場上棋子多的獲勝\n\n");
    printf("[操作]  落子時輸入棋盤座標(如: E3), 輪到對方時輸入 R 悔棋,\n");
    printf("        輸入 P 可察看遊戲規則\n\n");
    printf("Press Enter to continue...\n");
    while (getchar() != '\n')
        ;
    return 0;
}

int search(int *const piece, const int color)
{
    int exist = 0;

    for (int i = 0; i < 64; i++)
    {
        if (piece[i] == 1 || piece[i] == -1)
            continue; //* not empty
        if (piece[i] == 2 * color || piece[i] == -2 * color)
            piece[i] == 0;
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                for (int row = i / 8 + 1 + dx, col = i % 8 + 1 + dy; BOUNDARY(row, col); row += dx, col += dy)
                {
                    if (piece[row * 8 + col - 9] != -color)
                        break;
                    else if (BOUNDARY(row + dx, col + dy) && piece[(row + dx) * 8 + col + dy - 9] == color)
                    {
                        piece[i] = 2 * color;
                        exist = 1;
                        goto finish;
                    }
                }
            }
        }
    finish:;
    }
    return exist;
}

int gui(int *const piece)
{
    system("clear");
    int score_black = 0, score_white = 0;
    printf("\033[0m\n   [黑白棋]\n\n");
    printf("    1 2 3 4 5 6 7 8\n");
    printf("  ┌─────────────────┐\n");
    for (int i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
            printf(" %c| ", i / 8 + 'A');

        switch (piece[i])
        {
        case 1:
            printf("○ ");
            score_black++;
            break;
        case -1:
            printf("● ");
            score_white++;
            break;
        case 2:
        case -2:
            printf("┼ ");
            break;
        default:
            printf("┼ ");
        }
        if (i % 8 == 7)
            printf("│\n");
    }
    printf("  └─────────────────┘\n");
    printf("黑棋: %2d | 白棋:%2d \n", score_black, score_white);
    printf("- - - - - - - - - - - - - -\n");
    return score_black == score_white ? 0 : (score_black > score_white ? 1 : -1);
}

int info(int *const piece, const int sign, const int color)
{
    static int cnt[2] = {0};
    for (int i = 0; i < 2; i++)
        if (cnt[i] > 10)
        {
            printf("bad");
            cnt[i] = 0;
            return 0;
        }
    switch (sign >= 64 ? sign - 80 : sign)
    {
    case -1:
        printf("%s方悔棋!!\n", color == 1 ? "黑" : "白");
        break;
    case -2:
        printf("無效操作!!\n");
        cnt[0]++;
        break;
    case -3:
        introduce();
        gui(piece);
        cnt[2]++;
        break;

    default:
        if (sign < 64)
            printf("%s方: [%c%c]\n", color == 1 ? "白" : "黑", sign / 8 + 'A', sign % 8 + '1');
        else
            printf("%s方: [%c%c]\n%s方無落子位置!!\n", color == 1 ? "黑" : "白", sign / 8 - 10 + 'A', sign % 8 + '1', color == 1 ? "白" : "黑");
        cnt[0] == cnt[1] == 0;
        break;
    }
    return 0;
}

int retract(int *const piece, int *const piece_last)
{
    int sign = -2;
    for (int i = 0; i < 64; i++)
    {
        if (piece[i] != piece_last[i])
        {
            sign = -1;
            piece[i] = piece_last[i];
        }
    }
    return sign;
}

int place(int sockfd,int *const piece, int *const piece_last, const int color,const int order)
{
    int row = 0, col = 0;
    int n;
    char ch1, ch2;
    int location;
    char sendline[MAXLINE], recvline[MAXLINE];
    printf("現在是%s方的回合\n", color == 1 ? "黑" : "白");
    if(order == 1 ){
    	printf("you are the black player\n");
    }else if(order == -1){
    	printf("you are the white player\n");
    }else{
    	printf("you are the audience\n");
    }
    
    // fflush(stdin);
    if(order==color){
    	scanf(" %c", &ch1);
    	printf("row:%c\n", ch1);

   	 if (ch1 == 'R' || ch1 == 'r')
   	     return -1;
   	 if (ch1 == 'P' || ch1 == 'p')
   	     return -3;
   	 if (ch1 >= 'A' && ch1 <= 'H')
  	      row = ch1 + 1 - 'A';
  	  else if (ch1 >= 'a' && ch1 <= 'h')
  	      row = ch1 + 1 - 'a';
  	  else if (ch1 >= '1' && ch1 <= '8')
  	      row = ch1 - '0';
        
  	  scanf(" %c", &ch2);
  	  printf("col:%c\n", ch2);
  	  if (ch2 >= 'A' && ch2 <= 'H')
   	     col = ch2 + 1 - 'A';
 	   else if (ch2 >= 'a' && ch2 <= 'h')
  	      col = ch2 + 1 - 'a';
  	  else if (ch2 >= '1' && ch2 <= '8')
   	     col = ch2 - '0';

    // printf("%d %d\n", row, col);
    // while (getchar())
    //     ;

  	 if (!BOUNDARY(row, col))
       		 return -2;

   	 location = row * 8 + col - 9;
   	 if (piece[location] == 1 || piece[location] == -1){
       	 	return -2;
       	 }	
  	 if (piece[location] == 0 || piece[location] == -2 * color){
        	return -2;
        }
        	sprintf(sendline, "%d", row);
       		printf("Send: %s\n", sendline);
        	Write(sockfd, sendline, strlen(sendline));
        	
        	sprintf(sendline, "%d",col);
       		printf("Send: %s\n", sendline);
        	Write(sockfd, sendline, strlen(sendline));		
        	
	}else if(order!=color){
		if((n = Read(sockfd, recvline, MAXLINE)) == 0){
			printf("nothing\n");
		} 
    		else{
        		recvline[n] = '\0';
        		
        		row = recvline[0] - '0';
        		
        		printf("row:%d\n",row);
    		}
        	if((n = Read(sockfd, recvline, MAXLINE)) == 0){
        		printf("nothing\n");
        	} 
    		else{
        		recvline[n] = '\0';
        		printf("recv: %s\n", recvline);
        		col = recvline[0] - '0';
        		
        		printf("col:%d\n",col);
    		}
    		location = row * 8 + col - 9;
	}
    	retract(piece_last, piece);
   	piece[location] = color;

    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
        {
            if (dx == 0 && dy == 0)
                continue;

            for (int rowp = row + dx, colp = col + dy; BOUNDARY(rowp, colp); rowp += dx, colp += dy)
            {
                if (piece[rowp * 8 + colp - 9] != -color)
                    break;
                else if (BOUNDARY(rowp + dx, colp + dy) && piece[(rowp + dx) * 8 + colp + dy - 9] == color)
                    for (; rowp != row || colp != col; rowp -= dx, colp -= dy)
                        piece[rowp * 8 + colp - 9] = color;
            }
        }
        
        
    return location;
}


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
    



    
    clear_recv_send(recvline, sendline);
    if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
    else{
        recvline[n] = '\0';
        printf("recv: %s\n", recvline);
    }
    stage1: printf("===========================stage 1===============================\n"); // stage 1: choose login or register
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
                    // printf("debug ok\n");
                    sprintf(sendline, "ack");
                    Write(sockfd, sendline, strlen(sendline));
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
                    // printf("debug ok\n");
                    sprintf(sendline, "ack");
                    Write(sockfd, sendline, strlen(sendline));
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
        printf("recv: %s\n", recvline); //****possible probliem*********
        
    }
    scanf("%d", &mode); // change the enter string to int
    // ******  testing *******    
    if(mode == 0){
    game: printf("START\n");
    	int order=0;
    	
    	printf("start to test\n");
    	sprintf(sendline, "%s", "0");
        printf("Send: %s", sendline);
        Write(sockfd, sendline, strlen(sendline));
        
    	if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
        else{
            
            recvline[n] = '\0';
            printf("my color will be: %s\n", recvline);
            if(strcmp(recvline, "1")==0){
            	order=1;
            }else if(strcmp(recvline, "2")==0){
          
                order=-1;
            }else if(strcmp(recvline,"3")==0){
            	order=10;
            }
            start:
	    int piece[64] = {0};
    	    int piece_last[64];
    	    int color = 1, sign = -1;
    	    piece[27] = piece[36] = -1;
    	    piece[28] = piece[35] = 1;
    	    
            introduce();
    	    gui(piece);
    	    search(piece, color);
    	  //*********************************order1*****************************************  
    	    if(order==1){
    	    	
    	    	do
   		{
        		gui(piece);
        		if (sign <= -2)
            			info(piece, sign, color);
        		sign = place(sockfd,piece, piece_last, color,order);
        		gui(piece);
        		if (sign == -1)
            			sign = -5;
    		} while (sign <= -1);
    		color *= -1;
    		while (1)
    		{
        		if (search(piece, color *= 1) || (sign += 80, search(piece, color *= -1)))
        		{
            			do
            			{
                			gui(piece);
                			info(piece, sign, color);
                			
                			sign = place(sockfd,piece, piece_last, color,order);
                			gui(piece);
                			if (sign == -1)
                    			sign = retract(piece, piece_last);
            			} while (sign <= -2);
        		}
        		else
            		break;
        		color *= -1;
    		}
    int outcome = gui(piece);
    printf("%s方: [%c%c]\n", color == 1 ? "黑" : "白", sign / 8 + 'A', sign % 8 + '1');
    printf("遊戲結束, %s\n", outcome == 0 ? "平局!!" : (outcome == 1 ? "黑方勝!!" : "白方勝!!"));
    char c;
    for (fflush(stdin); (c != getchar()) != '0' && c != 'n' && c != 'N';)
        if (c == 1 || c == 'y' || c == 'Y')
            goto start;

    printf("Press Enter to continue...\n");
    while (getchar() != '\n')
        ;
    	    }
    //*********************************order2*****************************************
    	    else if(order==-1||order==10){
    	    	do
   		{
        		gui(piece);
        		if (sign <= -2)
            			info(piece, sign, color);
        		sign = place(sockfd,piece, piece_last, color,order);
        		gui(piece);
        		if (sign == -1)
            			sign = -5;
    		} while (sign <= -1);
    		color *= -1;
    		while (1)
    		{
        		if (search(piece, color *= 1) || (sign += 80, search(piece, color *= -1)))
        		{
            			do
            			{
                			gui(piece);
                			info(piece, sign, color);
                			
                			sign = place(sockfd,piece, piece_last, color,order);
                			gui(piece);
                			if (sign == -1)
                    			sign = retract(piece, piece_last);
            			} while (sign <= -2);
        		}
        		else
            		break;
        		color *= -1;
    		}
    int outcome = gui(piece);
    printf("%s方: [%c%c]\n", color == 1 ? "黑" : "白", sign / 8 + 'A', sign % 8 + '1');
    printf("遊戲結束, %s\n", outcome == 0 ? "平局!!" : (outcome == 1 ? "黑方勝!!" : "白方勝!!"));
    char c;
    for (fflush(stdin); (c != getchar()) != '0' && c != 'n' && c != 'N';)
        if (c == 1 || c == 'y' || c == 'Y')
            goto start;

    printf("Press Enter to continue...\n");
    while (getchar() != '\n')
        ;
    	    }

        }
    	
    }
    else if(mode == 1){
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
                goto game;
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
                // postpone untill another player enter the room
                n = Read(sockfd, recvline, MAXLINE);
                recvline[n] = '\0';
                printf("recv: %s\n", recvline);
                
                goto game;
            }
        }
    }
    else if(mode == 3){
        sprintf(sendline, "%s", "3");
        printf("Send: %s\n", sendline);
        Write(sockfd, sendline, strlen(sendline));
        if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
        else{
            recvline[n] = '\0';
            printf("recv: %s\n", recvline);
            int room_id;
            scanf("%d", &room_id);
            sprintf(sendline, "%d", room_id);
            printf("Send: %s\n", sendline);
            Write(sockfd, sendline, strlen(sendline));
            if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
            else{
                recvline[n] = '\0';
                printf("recv: %s\n", recvline);
                if(strcmp(recvline,"Cannot find the room.\nPlease try again later") == 0){
                    goto stage3;
                }
                else{
                    printf("Enter the game\n");
                    
                   
                    goto game;

                }
            }

        }
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

    stage4: printf("===========================stage 4===============================\n"); // stage 4: game start
    
    board_t game_board;
    init_board(game_board);
    int my_color = 0;
    // check the color
    clear_recv_send(recvline, sendline);
    if((n = Read(sockfd, recvline, MAXLINE)) == 0) goto terminate_prematurely;
    else{
        recvline[n] = '\0';
        printf("recv: %s\n", recvline);
        if(strcmp(recvline, "You are player1, your color is black") == 0){
            my_color = 1;
        }
        else if(strcmp(recvline, "You are player2, your color is white") == 0){
            my_color = -1;
        }
        else{
            printf("Invalid color\n");
            goto stage4;
        }
    }
    
    




    // normal return
    return;

    terminate_prematurely: printf("Server terminated prematurely\n");
        return;
}



int main(int argc, char **argv)
{

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
    
    printf("hold on\n");
    return 0;

}