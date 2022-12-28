#include <stdio.h>
#include <stdbool.h>

#define SIZE 5
#define TYPE 4

typedef struct Coord {
  int x;
  int y;
} Coord;

char player = 'x', opponent = 'o';

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

// This function returns true if there are moves remaining on the board. It returns false if
// there are no moves left to play.
bool isMovesLeft(char board[SIZE][SIZE]) {
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      if (board[i][j] == '_')
        return true;
  return false;
}

int evaluate(char board[SIZE][SIZE]) {
  int col, row, i, consecutive;

  // Check for ROW
  for(row = 0; row < SIZE; row++) {
    consecutive = 1;
    for(col = 0; col < SIZE - 1; col++) {
      consecutive = (board[row][col] != '_' && board[row][col] == board[row][col + 1]) ? consecutive + 1 : 1;
      if(consecutive == TYPE) {
        if(board[row][col] == player) return 10;
        else if(board[row][col] == opponent) return -10;
      }
    }
  }

  //   Check for COLUMN
  for(col = 0; col < SIZE; col++) {
    consecutive = 1;
    for(row = 0; row < SIZE - 1; row++) {
      consecutive = (board[row][col] != '_' && board[row][col] == board[row + 1][col]) ? consecutive + 1 : 1;
      if(consecutive == TYPE) {
        if(board[row][col] == player) return 10;
        else if(board[row][col] == opponent) return -10;
      }
    }
  }

  // Check for DIAGONALS
  // + RIGHT TOP -> LEFT BOTTOM
  for(i = 0; i < SIZE; i++) {
    consecutive = 1;
    for(row = 0, col = i; col > 0; col--, row++) {
      consecutive = (board[row][col] != '_' && board[row][col] == board[row + 1][col - 1]) ? consecutive + 1 : 1;
      if(consecutive == TYPE) {
        if(board[row][col] == player) return 10;
        else if(board[row][col] == opponent) return -10;
      }
    }
  }

  for(i = 1; i < SIZE; i++) {
    consecutive = 1;
    for(col = SIZE - 1, row = i; row < SIZE - 1; col--, row++) {
      consecutive = (board[row][col] != '_' && board[row][col] == board[row + 1][col - 1]) ? consecutive + 1 : 1;
      if(consecutive == TYPE) {
        if(board[row][col] == player) return 10;
        else if(board[row][col] == opponent) return -10;
      }
    }
  }

  // + LEFT TOP -> BOTTOM RIGHT
  for(i = 0; i < SIZE; i++) {
    consecutive = 1;
    for(row = 0, col = i; col < SIZE - 1; col++, row++) {
      consecutive = (board[row][col] != '_' && board[row][col] == board[row + 1][col + 1]) ? consecutive + 1 : 1;
      if(consecutive == TYPE) {
        if(board[row][col] == player) return 10;
        else if(board[row][col] == opponent) return -10;
      }
    }
  }

  for(i = 1; i < SIZE; i++) {
    consecutive = 1;
    for(row = i, col = 0; row < SIZE - 1; col++, row++) {
      consecutive = (board[row][col] != '_' && board[row][col] == board[row + 1][col + 1]) ? consecutive + 1 : 1;
      if(consecutive == TYPE) {
        if(board[row][col] == player) return 10;
        else if(board[row][col] == opponent) return -10;
      }
    }
  }

  return 0;
}

// This is the minimax function. It considers all
// the possible ways the game can go and returns the value of the board
int minimax(char board[SIZE][SIZE], int depth, bool isMax, int alpha, int beta) {
  int score = evaluate(board);
  if(score == 10)                   return score;
  if(score == -10)                  return score;
  if(isMovesLeft(board) == false)   return 0;

  int best = isMax ? 0 : 1;

  // Traverse all cells
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      // Check if cell is empty
      if (board[i][j] == '_') {
        // Make the move
        board[i][j] = isMax ? player : opponent;

        // Call minimax recursively and choose the maximum value
        best = (isMax ? max : min)(best, minimax(board, depth + 1, !isMax, alpha, beta));

        if(isMax) alpha = max(alpha, best);
        else beta = min(beta, best);

        // Undo the move
        board[i][j] = '_';

        // Alpha - beta pruning
        if(beta <= alpha) break;
      }
    }
  }
  return best;
}

// This will return the best possible move for the player
Coord findBestMove(char board[SIZE][SIZE]) {
  int bestVal = -10000;
  Coord bestMove;
  bestMove.x = -1;
  bestMove.y = -1;

  // Traverse all cells, evaluate minimax function for
  // all empty cells. And return the cell with optimal value.
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      // Check if cell is empty
      if (board[i][j] == '_') {
        printf("Row = %d Col = %d\n", i, j);

        // Make the move
        board[i][j] = player;
        // compute evaluation function for this move.
        int moveVal = minimax(board, 0, false, -10000, 10000);

        // Undo the move
        board[i][j] = '_';

        // If the value of the current move is
        // more than the best value, then update bestMove
        if (moveVal > bestVal) {
          bestMove.x = i;
          bestMove.y = j;
          bestVal = moveVal;
        }
      }
    }
  }

  printf("The value of the best Move is : %d\n\n", bestVal);
  return bestMove;
}

// Driver code
int main() {
  char board[SIZE][SIZE] = {
//    "__o__",
//    "oxx__",
//    "__x__",
//    "o____",
//    "__x__"
    // { '_', '_', 'o', '_', 'o' },
    // { 'x', 'o', 'x', 'x', 'o' },
    // { 'o', 'o', 'x', '_', 'x' },
    // { 'x', '_', 'o', 'o', 'x' },
    // { '_', '_', '_', '_', '_' },

    { '_', 'x', 'x', 'x', 'o' },
    { 'o', 'x', '_', 'x', '_' },
    { '_', 'o', '_', 'o', '_' },
    { '_', '_', 'o', '_', '_' },
    { '_', '_', '_', '_', '_' },

//    { '_', 'x', 'o', 'x', 'o' },
//    { 'o', 'o', 'o', 'x', 'o' },
//    { '_', 'o', 'x', 'o', '_' },
//    { 'o', 'x', 'o', 'x', 'x' },
//    { 'o', 'x', 'x', 'x', '_' },
    // { '_', '_', '_', '_', '_', '_' },

    //  { 'o', 'x', 'x', 'o', 'o', 'x', '_', '_' },
    //  { 'o', 'o', 'x', 'x', 'o', '_', '_', '_' },
    //  { '_', 'o', 'o', 'o', 'o', '_', '_', '_' },
    //  { '_', '_', 'o', 'x', '_', '_', '_', '_' },
    //  { '_', '_', '_', 'x', '_', '_', '_', '_' },
    //  { '_', '_', '_', '_', '_', '_', '_', '_' },
    //  { '_', '_', '_', '_', '_', '_', '_', '_' },
    //  { '_', '_', '_', '_', '_', '_', '_', '_' },
  };
  Coord bestMove = findBestMove(board);

  printf("The Optimal Move is :\n");
  printf("ROW: %d COL: %d\n\n", bestMove.x, bestMove.y );
  return 0;
}
