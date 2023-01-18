#include "config.h"
#include "algo.h"

int checkVerticalLine(char board[BOARD_S][BOARD_S], char turn, int x, int y) {
  int j = y + 1, count = 1;

  while (board[x][j] == turn && j < BOARD_S) {
    count++;
    if (count == 5) return SUCCESS;
    j++;
  }

  j= y - 1;
  while (board[x][j] == turn && j >= 0) {
    count++;
    if (count == 5) return SUCCESS;
    j--;
  }
  return FAILURE;
}

int checkHorizontalLine(char board[BOARD_S][BOARD_S], char turn, int x, int y) {
  int i = x + 1, count = 1;

  while (board[i][y] == turn && i < BOARD_S) {
    count++;
    if (count == 5) return SUCCESS;
    i++;
  }

  i = x - 1;
  while (board[i][y] == turn && i >= 0) {
    count++;
    if (count == 5) return SUCCESS;
    i--;
  }
  return FAILURE;
}

int checkDiagonallyLine1(char board[BOARD_S][BOARD_S], char turn, int x, int y) {
  int i = x + 1, j = y - 1,count = 1;

  while (board[i][j] == turn && j >= 0 && i < BOARD_S) {
    count++;
    if (count == 5) return SUCCESS;
    i++;
    j--;
  }

  i = x - 1;
  j = y + 1;
  while (board[i][j] == turn && i >= 0 && j < BOARD_S) {
    count++;
    if (count == 5) return SUCCESS;
    i--;
    j++;
  }
  return FAILURE;
}

int checkDiagonallyLine2(char board[BOARD_S][BOARD_S], char turn, int x, int y) {
  int i = x + 1, j = y + 1, count = 1;

  while (board[i][j] == turn && j < BOARD_S && i < BOARD_S) {
    count++;
    if (count == 5) return SUCCESS;
    i++;
    j++;
  }

  i = x - 1;
  j = y - 1;
  while (board[i][j] == turn && i >= 0 && j >= 0) {
    count++;
    if (count == 5) return SUCCESS;
    i--;
    j--;
  }
  return FAILURE;
}

int checkWinning(char board[BOARD_S][BOARD_S], char turn, int x, int y) {
  if (
    checkVerticalLine   (board, turn, x, y) == 1 ||
    checkHorizontalLine (board, turn, x, y) == 1 ||
    checkDiagonallyLine1(board, turn, x, y) == 1 ||
    checkDiagonallyLine2(board, turn, x, y) == 1
    )
    return SUCCESS;
  return FAILURE;
}