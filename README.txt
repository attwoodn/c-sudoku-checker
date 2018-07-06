Group members:
	Noah Attwood 	B00718872
		contribution: Taking user input and parsing it into a 9x9 grid. Implemented error messages during parsing. Created initial program structure and implemented passing struct to worker thread methods.  

	Jerrett DeMan	B00713718
		contribution: implemented sudoku solution validation by checking each row, column, and sub-grid in separate threads. Implemented checking the results of the puzzle validation and printing any errors found. 


Instructions to compile:
	gcc -o exec sudoku_checker.c -lpthread -std=c        (note: -std=c may be omitted depending on the system)