/* Picks a random line from input file and prints it to the terminal
 * Built using custom TeFiEd library (see github ADBeta)
 *
 * Pass filename after exec name to use. eg linePicker test.txt
 *
 * Last Modified 16 Apr 2022
 * ADBeta
*/

#include <ctime> 
#include <iostream>
#include "TeFiEd.hpp"

int main(int argc, char *argv[]) {
	//Complain if no args passed
	if(argc == 1) {
		std::cout << "No arguments given! exiting" << std::endl;
		return 1;
	}
	
	//Complain if too many args given
	if(argc > 2) {
		std::cout << "Too many arguments given! exiting" << std::endl;
		return 1;
	}

	//Create TeFiEd object using name passed from args
	TeFiEd inputFile(argv[1]);
	
	//Read the input file into RAM. Exit if fail
	if(inputFile.read() == EXIT_FAILURE) return 1;
	
	//Generate random seed based on time
	srand((unsigned int)time(0)); 
	
	//Generate random number based on the linecount. 1 is minimum
	size_t selLine = (rand() % inputFile.lines()) + 1;
	
	//Print output
	std::cout << selLine << " | " << inputFile.getLine(selLine) << std::endl;

	return 0;
}
