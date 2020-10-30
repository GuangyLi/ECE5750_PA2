/* C/C++ program to solve n Queen Problem using 
backtracking using parallel algorithm */
#include <pthread.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BILLION 1000000000L 

int best_profit, sol_num, *best_board; // global variables used by all threads
struct timespec time2;
pthread_barrier_t barrier;
pthread_mutex_t sum_lock, best_lock; // lock to prevent write to sum and best case at the same time

// Data structure that passed into each processor
typedef struct {
    int n, p, pid, **boards;
} GM;

/////////////////////////////////////Utility Functions/////////////////////////////////////

/* A utility function to print solution in 2D visual representation */
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

/* Copies src array into dest array */
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
  
  //i represents column number whereas board[i] represent row number
  for (i = 0; i < n; i++) prof += abs(i - board[i]); 
  
  return prof;
}
    
/* Returns true if the board satisfies the n queens problem if a queen 
   were to be inserted at row r and column col */
bool isSafe(int r, int *board, int col) 
{ 
  int i;
  // return false if two queens share the same column
  for (i = 0; i < col; i++){
    if ((board[i]==r) || (col-i==abs(board[i]-r))) return false;
  }
  return true;
} 

/////////////////////////////////////Recursive and parallel Functions/////////////////////////////////////

// The recursive function that solve the nqueen problem using backtracking
void solvenq(int n, int *board, int col, int pid) 
{
  // base case for when all queens have been inserted
  if (col == n) {
	  
    // Prevent write to sol_num at the same time
    pthread_mutex_lock(&sum_lock);
    sol_num++;
    pthread_mutex_unlock(&sum_lock);
	
    // Copy the current board to the memory allocated to best_board if the current profit is larger than the store best case
    int temp;
    temp = totalProfit(n, board);
    if (temp > best_profit) {
      // Prevent write to best case at the same time, not necessary currently since this does not occur frequently
      pthread_mutex_lock(&best_lock);
      best_profit = temp;
      copy(n, board, best_board);
      pthread_mutex_unlock(&best_lock);
    }
    return;
  }
  
  // Iterate from row 0 to n to place all acceptable queens on the current column
  int i;
  for (i = 0; i < n; i++) {
    board[col] = i;
    // if no two queens threaten each other
    if (isSafe(i, board, col)) solvenq(n, board, col+1, pid);
  }
}

// The parallel function that assign tasks to each processor
void *
psolvenq(void *varg) {
  GM *arg = varg;
  int pid, n, p, **all_boards, i, j;
  n = arg->n;
  p = arg->p;
  pid = arg->pid;
  all_boards = arg->boards;
  
  pthread_barrier_wait(&barrier);
  
  // take timestamp 2 (only in thread 0)
  if (pid == 0) clock_gettime(CLOCK_MONOTONIC, &time2);

  for (i = pid; i < n; i=i+p) {
    // place queen on current square
    all_boards[pid][0] = i;
    // recur for next row
    solvenq(n, all_boards[pid], 1, pid);
  }
}


/////////////////////////////////////Main Code/////////////////////////////////////
// Calculates execution time and perform nqueen calculation
int
main(int argc, char **argv) {
  struct timespec time1, time3, time4;
  double setup_time, exec_time, finish_time;
  int i, j, p, n, **all_boards; // i, j, :iterative index, p: # of processor, n: number of queens & board dimensions

  if(argc != 3) {
      printf("Usage: nqueens_parallel n p\nAborting...\n");
      exit(0);
  }
  n = atoi(argv[1]);
  p = atoi(argv[2]);

  // Allocate memory to the 2D array that stores aboard and shared by all processors
  all_boards = (int **) malloc(p * sizeof(int *));
  for(i = 0; i < p; i++) {
    all_boards[i] = (int *) malloc(n * sizeof(int));
    for(j = 0; j < n; j++) {
        all_boards[i][j] = 0;
    }
  }

  // initialize board to each processor and allocate its memory
  best_board = (int *) malloc(n * sizeof(int));
  for (i = 0; i < n; i++) {
      best_board[i] = 0;
  }

  // initialize solution number 
  sol_num = 0;
  
  // initialize pthread barrier to synchronize p threads
  pthread_barrier_init(&barrier, NULL, p);
  
  // take timestamp 1
  clock_gettime(CLOCK_MONOTONIC, &time1);
  
  // allocate memory for thread
  pthread_t *threads = malloc(p * sizeof(threads));
  for (i = 0; i < p; i++) {

    // struct to pass in input (ptr to all matrix and vectors)
    GM *arg = malloc(sizeof(*arg));
    arg->n = n;
    arg->p = p;
    arg->pid = i;
    arg->boards = all_boards;

    // assign workload to thread, each thread seems to have acces to the full data
    pthread_create(&threads[i], NULL, psolvenq, arg);
  }

  for(i = 0; i < p; i++) pthread_join(threads[i], NULL);
    
  // take timestamp 3
  clock_gettime(CLOCK_MONOTONIC, &time3);
  
  // calculate setup and execution times
  setup_time = BILLION * 
    (time2.tv_sec - time1.tv_sec) + (time2.tv_nsec - time1.tv_nsec);
  setup_time = setup_time / BILLION;
  
  exec_time = BILLION * 
    (time3.tv_sec - time2.tv_sec) + (time3.tv_nsec - time2.tv_nsec);
  exec_time = exec_time / BILLION;
  
  // prints information calculated in problem
  printf("There are %i solutions and the solution with the highest profit is: \n\n", sol_num);
  printSolution(n, best_board);
  printf("Profit: %i\n", best_profit);
  printf("Setup time: %lf seconds\n", setup_time);
  printf("Execution time: %lf seconds\n", exec_time);
  
  // take timestamp 4
  clock_gettime(CLOCK_MONOTONIC, &time4);
  
  // calculate finish time
  finish_time = BILLION * 
    (time4.tv_sec - time3.tv_sec) + (time4.tv_nsec - time3.tv_nsec);
  finish_time = finish_time / BILLION;
  
  printf("Finish time: %lf seconds\n\n", finish_time);

  // frees all allocated memory
  for (i = 0; i < p; i++) {
    free(all_boards[i]);
  }
  free(all_boards);
  free(best_board);
  free(threads);

  return 0;
}
