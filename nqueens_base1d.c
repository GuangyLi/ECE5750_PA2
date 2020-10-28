/* C/C++ program to solve n Queen Problem using 
backtracking */
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BILLION 1000000000L 

int best_profit, *best_board, sol_num;

/* A utility function to print solution */
void printSolution(int n, int *board) 
{ 
  int i,j;
	for (i = 0; i < n; i++) { 
		for (j = 0; j < n; j++) { 
      if (j == board[i]) printf("1 "); 
      else printf("0 "); 
    }
		printf("\n"); 
	} 
  printf("\n");
} 

/* Copies src n x n matrix into dest n x n matrix */
void copy(int n, int *src, int *dest)
{
  int i;
  for (i = 0; i < n; i++) dest[i] = src[i];
}

/* Calculates total profit of a board using formula from PA2 handout */
int totalProfit(int n, int *board)
{
  int i, prof;
  prof = 0;
  for (i = 0; i < n; i++) prof += abs(i - board[i]);
  
  return prof;
}
    
/* Returns true if the board satisfies the n queens problem if a queen 
   were to be inserted at row r and column c */
bool isSafe(int r, int *board, int c) 
{ 
  int i;
	// return false if two queens share the same column
	for (i = 0; i < c; i++){
		if ((board[i]==r) || (c-i==abs(board[i]-r))) return false;
  }
	return true;
} 

void solvenq(int n, int *board, int c) 
{
  // base case for when all queens have been inserted
  if (c == n) {
    sol_num++;
    int temp;
    temp = totalProfit(n, board);
    if (temp > best_profit) {
      best_profit = temp;
      copy(n, board, best_board);
    }
    return;
  }
  int i;
  for (i = 0; i < n; i++) {
    board[c] = i;
		// if no two queens threaten each other
		if (isSafe(i, board, c)) solvenq(n, board, c+1);
	}
}

int main(int argc, char **argv) 
{ 
  struct timespec start, end;
  double time;
  int n, i, j, *board;
  if(argc != 2) {
        printf("Usage: nqueens_baseline n\nAborting...\n");
        exit(0);
    }
  n = atoi(argv[1]);
  
  sol_num = 0;
  
  // initialize boards and allocate their memory
  board = (int *) malloc(n * sizeof(int));
  best_board = (int *) malloc(n * sizeof(int));
  for (i = 0; i < n; i++) {
      board[i] = 0;
      best_board[i] = 0;
  }

  // calls program to run while tracking execution time
	clock_gettime(CLOCK_MONOTONIC, &start);
  solvenq(n, board, 0);
  clock_gettime(CLOCK_MONOTONIC, &end);
    
  time =
  BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  time = time / BILLION;
    
  // prints information calculated in problem
  printf("Elapsed: %lf seconds\n\n", time);
  printf("There are %d solutions and the solution with the highest profit is: \n\n", sol_num);
  printSolution(n, best_board);
  printf("Profit: %i\n", best_profit);
  
  // frees all allocated memory
  free(board);
  free(best_board);
	return 0; 
} 
