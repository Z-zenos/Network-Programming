#include "algo.h"
#include "config.h"

int ROW = 8, COLUMN = 8;

int checkVerticalLine(int board[COLUMN][ROW], int player, int x, int y) {
  int j = y + 1, count = 1;

  while (board[x][j] == player && j < ROW) {
    count++;
    if (count == 5) return SUCCESS;
    j++;
  }

  j= y - 1;
  while (board[x][j] == player && j >= 0) {
    count++;
    if (count == 5) return SUCCESS;
    j--;
  }
  return FAILURE;
}

int checkHorizontalLine(int board[COLUMN][ROW], int player, int x, int y) {
  int i = x + 1, count = 1;

  while (board[i][y] == player && i < COLUMN) {
    count++;
    if (count == 5) return SUCCESS;
    i++;
  }

  i = x - 1;
  while (board[i][y] == player && i >= 0) {
    count++;
    if (count == 5) return SUCCESS;
    i--;
  }
  return FAILURE;
}

int checkDiagonallyLine1(int board[COLUMN][ROW], int player, int x, int y) {
  int i = x + 1, j = y - 1,count = 1;

  while (board[i][j] == player && j >= 0 && i < COLUMN) {
    count++;
    if (count == 5) return SUCCESS;
    i++;
    j--;
  }

  i = x - 1;
  j = y + 1;
  while (board[i][j] == player && i >= 0 && j < ROW) {
    count++;
    if (count == 5) return SUCCESS;
    i--;
    j++;
  }
  return FAILURE;
}

int checkDiagonallyLine2(int board[COLUMN][ROW], int player, int x, int y) {
  int i = x + 1, j = y + 1, count = 1;

  while (board[i][j] == player && j < COLUMN && i < ROW) {
    count++;
    if (count == 5) return SUCCESS;
    i++;
    j++;
  }

  i = x - 1;
  j = y - 1;
  while (board[i][j] == player && i >= 0 && j >= 0) {
    count++;
    if (count == 5) return SUCCESS;
    i--;
    j--;
  }
  return FAILURE;
}

int checkWinning(int board[COLUMN][ROW], int player, int x, int y) {
  if (
    checkVerticalLine   (board, player, x, y) == 1 ||
    checkHorizontalLine (board, player, x, y) == 1 ||
    checkDiagonallyLine1(board, player, x, y) == 1 ||
    checkDiagonallyLine2(board, player, x, y) == 1
    )
    return SUCCESS;
  return FAILURE;
}