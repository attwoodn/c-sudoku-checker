#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>


/* variable declarations */
// structure for passing data to threads
typedef struct {
	int row;
	int row_values[9];
} sudoku_thread_params;

// A value of 0 means the row was not valid. Each thread should set the 
// coresponding row index value to 1 if it determines the row is valid
int valid_row_arr[9] = {0};


/* function declarations */
void *check_sudoku_row(void *args){
	sudoku_thread_params *stp = (sudoku_thread_params *)args;
	printf("%d\n", stp->row);
	// sudoku row is valid, so mark it as correct
	// valid_row_arr[stp->row] = 1;
}


int main(void){
	//storage for thread IDs
	pthread_t tid_arr[9];

	// take user input from stdin (would allow for piping input from a file)
	// tokenize user input on space or \n. do some validation to ensure that all values are < 10, and num values == 81
	// store values into an 81 length array or 2D 9x9 array to store the sudoku puzzle values
	// Set row_values array in sudoku_thread_params struct for each row of 9 values
	
	// create the worker threads and run them
	for(int i = 0; i < 9; i++){
		sudoku_thread_params *data = (sudoku_thread_params *) malloc(sizeof(sudoku_thread_params));
		data->row = i;
		if (pthread_create(&tid_arr[i], NULL, &check_sudoku_row, (void *)data) != 0){
			printf("Failed to create thread number %d\n", i);
        	exit(1);
		}
	}

	// terminate the worker threads
	for(int i = 0; i < 9; i++){
		pthread_join(tid_arr[i], NULL);
	}

	// check that all sudoku rows were valid
	for(int i = 0; i < 9; i++){
		if(valid_row_arr[i] == 0){
			printf("Row %d is invalid\n", i+1);
		}
	}
}