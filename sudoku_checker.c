#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define INPUT_BUF_SIZE		20


/* variable declarations */
// structure for passing data to threads
typedef struct {
	int row;
	int row_values[9];
} sudoku_thread_params;

// A value of 0 means the row was not valid. Each thread should set the 
// coresponding row index value to 1 if it determines the row is valid
int valid_row_arr[9] = {0};

// variable for turning on and off debug print statements
int debug = 1;


/* function declarations */
void *check_sudoku_row(void *args){
	sudoku_thread_params *stp = (sudoku_thread_params *)args;
	if (debug) printf("%d\n", stp->row);
	// sudoku row is valid, so mark it as correct
	// valid_row_arr[stp->row] = 1;
}


int main(void){
	//storage for thread IDs
	pthread_t tid_arr[9];

	char *input_text = calloc(1, sizeof(char));
	char buffer[INPUT_BUF_SIZE];
	
	while(fgets(buffer, INPUT_BUF_SIZE , stdin)) {
		input_text = realloc(input_text, strlen(input_text) + strlen(buffer) + 1);

		if(input_text == NULL){
			printf("Failed to read input buffer");
			exit(1);
		}

		strcat(input_text, buffer);
	}

	if (debug) printf("text: \n%s\n\n\n", input_text);


	int col_index = 0, row_index = 0;
	int sudoku_grid[9][9]; 
	char * token = strtok(input_text, " \n");

	if (debug) printf("tokens:\n");
	while( token != NULL ) {
      	if (debug) printf( "%s\n", token);
    	// attempt to parse the string token to an integer
    	int cell_value;
		int convertSuccessful = sscanf(token, "%d", &cell_value);

		if (!convertSuccessful){
			printf("Error: value in cell at row %d and column %d had a non integer value (%s)\n", row_index+1, col_index+1, token);
			exit(1);
		}

		if (cell_value > 9 || cell_value < 0){
			printf("Error: value in cell at row %d and column %d had a value outside of the range 0-9 (%d)\n", row_index+1, col_index+1, cell_value);
			exit(1);
		}

		sudoku_grid[row_index][col_index] = cell_value;
		
		col_index ++;
		token = strtok(NULL, " \n");

		if(col_index == 9){
			if(row_index < 8){
				// end of a row of values. Reset row and increment column
				col_index = 0;
				row_index++;	
			} else if (token == NULL){
				// no more tokens and row/col are at their max value. Puzzle parsing is complete
				row_index++;
			} else {
				// row/col are both at their max value and there are more tokens. There are too many values to parse into a 9x9 grid
				printf("Error: too many values were in the sudoku puzzle\n");
				exit(1);
			}
		}
   }

   if(col_index != 9 || row_index != 9){
   		if (debug) printf("row: %d    col: %d\n", row_index, col_index);
   		printf("Error: not enough values were in the sudoku puzzle\n");
   		exit(1);
   }

   if (debug) printf("\n\n");

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