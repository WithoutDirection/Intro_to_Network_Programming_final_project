#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define maxsize = 8;
#define BOUNDARY(x, y) ((x) >= 1 && (x) <= 8 && (y) >= 1 && (y) <= 8)


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

int place(int *const piece, int *const piece_last, const int color)
{
    int row = 0, col = 0;
    char ch1, ch2;
    printf("現在是%s方的回合\n", color == 1 ? "黑" : "白");
    // fflush(stdin);

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

    int location = row * 8 + col - 9;
    if (piece[location] == 1 || piece[location] == -1)
        return -2;
    if (piece[location] == 0 || piece[location] == -2 * color)
        return -2;

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