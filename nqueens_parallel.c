/* C/C++ program to solve n Queen Problem using 
backtracking using parallel algorithm */
#include <pthread.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BILLION 1000000000L 

int best_profit, *sol_num, **best_board;

/* A utility function to print solution */
void printSolution(int n, int **board) 
{ 
  int i,j;
	for (i = 0; i < n; i++) { 
		for (j = 0; j < n; j++) 
			printf(" %i ", board[i][j]); 
		printf("\n"); 
	} 
  printf("\n");
} 

/* Copies src n x n matrix into dest n x n matrix */
void copy(int n, int **src, int **dest)
{
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      dest[i][j] = src[i][j];
  }
}

/* Calculates total profit of a board using formula from PA2 handout */
int totalProfit(int n, int **board)
{
  int i, j, prof;
  prof = 0;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      if (board[i][j])
        prof += abs(i - j);
  }
  return prof;
}
    
/* Returns true if the board satisfies the n queens problem if a queen 
   were to be inserted at row r and column c */
bool isSafe(int n, int **board, int r, int c) 
{ 
  int i, j;
	// return false if two queens share the same column
	for (i = 0; i < r; i++)
		if (board[i][c])
			return false;

	// return false if two queens share the same \ diagonal
	for (i = r, j = c; i >= 0 && j >= 0; i--, j--)
		if (board[i][j])
			return false;

	// return false if two queens share the same / diagonal
	for (i = r, j = c; i >= 0 && j < n; i--, j++)
		if (board[i][j])
			return false;

	return true;
} 

typedef struct {
    int n, p, pid;
} GM;

void solvenq(int n, int **board, int row, int pid) 
{
  // base case for when all queens have been inserted
  if (row == n) {
    sol_num[pid] = sol_num[pid] + 1;
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
		// if no two queens threaten each other
		if (isSafe(n, board, row, i)) {
			// place queen on current square
			board[row][i] = 1;
			// recur for next row
			solvenq(n, board, row+1, pid);
			// backtrack and remove queen from current square
			board[row][i] = 0;
		}
	}
}

void *
psolvenq(void *varg) {
  GM *arg = varg;
  int pid, n, p, **temp_board, i, j;
  n = arg->n;
  p = arg->p;
  pid = arg->pid;

  // initialize board to each processor and allocate its memory
  temp_board = (int **) malloc(n * sizeof(int *));
  for (i = 0; i < n; i++) {
      temp_board[i] = (int *) malloc(n * sizeof(int));
      for (j = 0; j < n; j++) {
          temp_board[i][j] = 0;
      }
  }

  for (i = pid; i < n; i=i+p) {
    // place queen on current square
    temp_board[0][i] = 1;
    // recur for next row
    solvenq(n, temp_board, 1, pid);
    // backtrack and remove queen from current square
    temp_board[0][i] = 0;
	}

  // frees all allocated memory
  for (i = 0; i < n; i++) {
    free(temp_board[i]);
  }
  free(temp_board);
}

int
main(int argc, char **argv) {
  struct timespec start, end;
  int i, j, p, n;  // i, j, :iterative index, p: # of processor, n: matrix dimension
  double time;     // time indicates execution time

  if(argc != 3) {
      printf("Usage: nqueens_parallel n p\nAborting...\n");
      exit(0);
  }
  n = atoi(argv[1]);
  p = atoi(argv[2]);

  // initialize board to each processor and allocate its memory
  best_board = (int **) malloc(n * sizeof(int *));
  for (i = 0; i < n; i++) {
      best_board[i] = (int *) malloc(n * sizeof(int));
      for (j = 0; j < n; j++) {
          best_board[i][j] = 0;
      }
  }

  // initialize solution number and best profit array that store each data in each processor
  sol_num = (int *) malloc(n * sizeof(int));
  best_profit = 0;
  for (i = 0; i < p; i++) {
    sol_num[i] = 0;
  }

  clock_gettime(CLOCK_MONOTONIC, &start);

  // allocate memory for thread
  pthread_t *threads = malloc(p * sizeof(threads));
  for (i = 0; i < p; i++) {

      // struct to pass in input (ptr to all matrix and vectors)
      GM *arg = malloc(sizeof(*arg));
      arg->n = n;
      arg->p = p;
      arg->pid = i;

      // assign workload to thread, each thread seems to have acces to the full data
      pthread_create(&threads[i], NULL, psolvenq, arg);
  }

  for(i = 0; i < p; i++)
      pthread_join(threads[i], NULL);
  clock_gettime(CLOCK_MONOTONIC, &end);
  free(threads);
  
  time = BILLION *(end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec);
  time = time / BILLION;

  int sol_total = 0;
  for (i = 0; i < n; i++) {
    sol_total = sol_total + sol_num[i];
  }
  
  // prints information calculated in problem
  printf("Elapsed: %lf seconds\n\n", time);
  printf("There are %d solutions and the solution with the highest profit is: \n\n", sol_total);
  printSolution(n, best_board);
  printf("Profit: %i\n", best_profit);
  
  // frees all allocated memory
  for (i = 0; i < n; i++) {
    free(best_board[i]);
  }
  free(best_board);

  return 0;
}
