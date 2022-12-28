#include <stdio.h>
#define COLUMN 8
#define ROW 8

int checkVerticalLine(int player,int x,int y,int board[COLUMN][ROW]) {
  int j,count=1;

  j=y+1;

  while (board[x][j] == player && j < ROW) {
    printf("%d\n",count);
    count++;
    if (count == 5) return 1;
    j++;
  }

  j=y-1;
  while (board[x][j] == player && j >= 0) {
    count++;
    if (count == 5) return 1;
    j--;
  }
  return 0;
}

int checkHorizontalLine(int player,int x,int y,int board[COLUMN][ROW]) {
  int i,count=1;

  i=x+1;
  while (board[i][y] == player && i < COLUMN) {
    count++;
    if (count == 5) return 1;
    i++;
  }

  i=x-1;
  while (board[i][y] == player && i >= 0) {
    count++;
    if (count == 5) return 1;
    i--;
  }
  return 0;
}

int checkDiagonallyLine1(int player,int x,int y,int board[COLUMN][ROW]) {
  int i,j,count=1;

  i=x+1;
  j=y-1;

  while (board[i][j] == player && j >= 0 && i < COLUMN) {
    count++;
    if (count == 5) return 1;
    i++;j--;
  }

  i=x-1;j=y+1;
  while (board[i][j] == player && i >= 0 && j < ROW) {
    count++;
    if (count == 5) return 1;
    i--;j++;
  }
  return 0;
}

int checkDiagonallyLine2(int player,int x,int y,int board[COLUMN][ROW]) {
  int i,j,count=1;

  i=x+1;
  j=y+1;

  while (board[i][j] == player && j < COLUMN && i < ROW) {
    count++;
    if (count == 5) return 1;
    i++;j++;
  }

  i=x-1;j=y-1;
  while (board[i][j] == player && i >= 0 && j >= 0) {
    count++;
    if (count == 5) return 1;
    i--;j--;
  }
  return 0;
}

int checkWinning(int player,int x,int y,int board[COLUMN][ROW]) {
  if (
    checkVerticalLine(player,x,y,board)==1 ||
    checkHorizontalLine(player,x,y,board)==1 ||
    checkDiagonallyLine1(player,x,y,board)==1 ||
    checkDiagonallyLine2(player,x,y,board)==1 )
    return 1;
  return 0;
}

void printBoard(int board[COLUMN][ROW]){
  int i, j;
  for (i = 0;i < COLUMN;i++) {
    for (j = 0;j < ROW;j++)
      printf("%d ", board[i][j]);
    printf("\n");
  }
}

 int main() {
   int komoku[COLUMN][ROW] = {
     {0, 0, 0, 1, 0, 0, 0, 0},
     {0, 2, 1, 1, 1, 1, 0, 0},
     {0, 1, 2, 1, 2, 2, 0, 0},
     {0, 2, 0, 2, 1, 2, 0, 0},
     {0, 0, 0, 0, 0, 1, 0, 0},
     {0, 0, 0, 0, 0, 0, 1, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
   };

   if (checkWinning(1,5,6,komoku) == 1)
     printf("Nguoi 1 win!\n");
   printf("Chua co gi !\n");
   return 1;
 }