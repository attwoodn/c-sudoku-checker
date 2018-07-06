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
	int section;
	int row;
	int column;
	int board[9][9];
    int result[9][9];
} sudoku_thread_params;

// A value of 0 means the part was not valid. Each thread should set the
// corresponding part index value to 1 if it determines the row is valid
int valid_row_arr[9] = {0};
int valid_column_arr[9] = {0};
int valid_3x3_section_arr[9] = {0};

// variable for turning on and off debug print statements
int debug = 0;


/* function declarations */

// comparision function used by quick sort
int cmpfunc (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

// Checks the validity of the row of the board specified in the sudoku_thread_params
void *check_sudoku_row(void *args){
	sudoku_thread_params *stp = (sudoku_thread_params *)args;

    // mark the row as valid till proven otherwise
    valid_row_arr[stp->row] = 1;

	// sort the row
    qsort(stp->board[stp->row], 9, sizeof(int), cmpfunc);

    // check that the sorted row is 1-9 in order
    for (int i = 0; i < 9; i++) {
        if (stp->board[stp->row][i] != i + 1) {
            valid_row_arr[stp->row] = 0;
            break;
        }
    }
}

// Checks the validity of the column of the board specified in the sudoku_thread_params
void *check_sudoku_column(void *args){
    sudoku_thread_params *stp = (sudoku_thread_params *)args;

    // mark the column as valid till proven otherwise
    valid_column_arr[stp->column] = 1;

    // copy the column into a 1D array
    int column[9];
    for (int i = 0; i < 9; i++)
        column[i] = stp->board[i][stp->column];

    // sort the column
    qsort(column, 9, sizeof(int), cmpfunc);

    // check that the sorted column is 1-9 in order
    for (int i = 0; i < 9; i++)
        if (column[i] != i + 1) {
            valid_column_arr[stp->column] = 0;
            break;
        }
}

// Checks the validity of the 3x3 section of the board specified in the sudoku_thread_params
void *check_sudoku_3x3_section(void *args){
	sudoku_thread_params *stp = (sudoku_thread_params *)args;

	int section[9];

	// parse the 3x3 section into a 1d array
	int num = 0;
	for(int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++){
			section[num] = stp->board[i + stp->row][j + stp->column];
			num++;
		}
	}

	// mark the section as valid till proven otherwise
	valid_3x3_section_arr[stp->section] = 1;

	// sort the section numbers
	qsort(section, 9, sizeof(int), cmpfunc);

	// check that the sorted section is 1-9 in order
	for (int i = 0; i < 9; i++)
		if (section[i] != i + 1) {
			valid_3x3_section_arr[stp->section] = 0;
			break;
		}
}


int main(void){
	//storage for thread IDs
	pthread_t tid_arr[27];

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
	
	// create the row worker threads and run them
	for(int i = 0; i < 9; i++){
		sudoku_thread_params *data = (sudoku_thread_params *) malloc(sizeof(sudoku_thread_params));
		memcpy(data->board, sudoku_grid, sizeof (int) * 9 * 9);
        data->row = i;
		if (pthread_create(&tid_arr[i], NULL, &check_sudoku_row, (void *)data) != 0){
			printf("Failed to create row thread number %d\n", i);
        	exit(1);
		}
	}

    // create the column threads and run them
    for(int i = 0; i < 9; i++){
        sudoku_thread_params *data = (sudoku_thread_params *) malloc(sizeof(sudoku_thread_params));
        memcpy(data->board, sudoku_grid, sizeof (int) * 9 * 9);
        data->column = i;
        if (pthread_create(&tid_arr[i+9], NULL, &check_sudoku_column, (void *)data) != 0){
            printf("Failed to create column thread number %d\n", i);
            exit(1);
        }
    }

	// create the 3x3 sections and run them
	int section = 0;
	for(int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++) {
			sudoku_thread_params *data = (sudoku_thread_params *) malloc(sizeof(sudoku_thread_params));
			memcpy(data->board, sudoku_grid, sizeof(int) * 9 * 9);
			data->column = j * 3;
			data->row = i * 3;
			data->section = section;
			if (pthread_create(&tid_arr[18 + section], NULL, &check_sudoku_3x3_section, (void *) data) != 0) {
				printf("Failed to create section thread number %d\n", section);
				exit(1);
			}
			section++;
		}
	}

	// wait for the worker threads
	for(int i = 0; i < 27; i++){
		pthread_join(tid_arr[i], NULL);
	}

	int returnValue = 0;

	// check that all sudoku rows were valid
	for(int i = 0; i < 9; i++){
		if(valid_row_arr[i] == 0){
			printf("Row %d is invalid\n", i+1);
			returnValue = 1;
		}
	}

    // check that all sudoku column were valid
    for(int i = 0; i < 9; i++){
        if(valid_column_arr[i] == 0){
            printf("Column %d is invalid\n", i+1);
			returnValue = 1;
        }
    }

    // check that all sudoku 3x3 sections were valid
    for(int i = 0; i < 9; i++){
        if(valid_3x3_section_arr[i] == 0){
            printf("Section %d is invalid\n", i+1);
			returnValue = 1;
        }
    }

    if (returnValue == 0) {
        printf("Puzzle is correctly solved\n");
	}

	return returnValue;
}