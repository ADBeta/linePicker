#include "TeFiEd.hpp"

#include <fstream>
#include <iostream>
#include <string>

/** System Functions **/
TeFiEd::TeFiEd(const char* inputfn) {
	m_filename = inputfn;
}

//Three Argument version of errorMsg
template <typename T1, typename T2, typename T3>
void TeFiEd::errorMsg(std::string func, T1 msg1, T2 msg2, T3 msg3) {
	//Print generic error message explaining where and what caused the error
	std::cerr << "Error: " << this->m_filename << ": " << func << ": ";
	
	//Print each message with period and newline
	std::cerr << msg1 << " " << msg2 << " " << msg3 << '.' << std::endl;
}

//Two argument version of errorMsg
template <typename T1, typename T2>
void TeFiEd::errorMsg(std::string func, T1 msg1, T2 msg2) {
	//Print generic error message explaining where and what caused the error
	std::cerr << "Error: " << this->m_filename << ": " << func << ": ";
	
	//Print each message with period and newline
	std::cerr << msg1 << " " << msg2 << '.' <<std::endl;
}

//Single argument version of errorMsg
template <typename T1>
void TeFiEd::errorMsg(std::string func, T1 msg1) {
	//Print generic error message explaining where and what caused the error
	std::cerr << "Error: " << this->m_filename << ": " << func << ": ";
	
	//Print each message with period and newline
	std::cerr << msg1 << '.' <<std::endl;
}

/** Low Level File Functions **/
void TeFiEd::flush() {
	//Empties out the vector and shrinks its size
	m_ramfile.clear();
	m_ramfile.shrink_to_fit();
}

size_t TeFiEd::bytes() {
	//Return number of bytes in the file
	size_t byteCount = 0;
	
	//Go through every element in the vector
	size_t vectElement = 0;
	while(vectElement < m_ramfile.size()) {
		//For each element, add the string size to the count.
		//NOTE <string>.size() returns number of bytes. it does not natively
		//understand unicode or multi-byte character sets, so this should
		//be a reliable method of getting bytes (as of 2022)
		byteCount += m_ramfile[vectElement].size();
	
		//NOTE Strings do not know about their terminating \n, so add 
		//1 byte to the count after every loop
		//This has been tested to agree with both Thunar and Nautilus 
		//file manager. may need rework later in time
		++byteCount;
	
		++vectElement;
	}
	
	return byteCount;
}

size_t TeFiEd::lines() {
	return m_ramfile.size();
}

void TeFiEd::resetAndClose() {
	//Private function. resets bit flags and closes the file
	//Clar flags
	m_file.clear();
	//Seek to 0th byte
	m_file.seekg(0, std::ios::beg);
	//Close file
	m_file.close();
}

/** Basic I/O Functions **/
//Read file into RAM
int TeFiEd::read() {
	//Reads the specified file into a RAM vector, up until certain RAM limit
	
	//Open file as read
	m_file.open(m_filename, std::ios::in);
	
	//Make sure file is open and exists
	if(m_file.is_open() == 0) {
		errorMsg("read", "File does not exist");
		return EXIT_FAILURE;
	}

	//TODO bytecount rework
	size_t byteCount = 0;

	//String containing current line of text
	std::string lineStr;
	
	//Get the next line in the stream, unless next line is EOF
	while(this->m_file.peek() != EOF) {
		//Copy current line string to var
		std::getline(this->m_file, lineStr);
		
		//Byte count, same methodology as getRAMBytes.
		//Add bytes to RAM byte counter
		byteCount += lineStr.size();
		//Add newline char per loop
		++byteCount;
		
		//Check that the next push won't overflow the size of the file
		if(byteCount > MAX_RAM_BYTES) {
			//Error message
			errorMsg("read", "File exceeds MAX_RAM_BYTES -", MAX_RAM_BYTES);
			
			//Return error status
			return EXIT_FAILURE;
		}
		
		//if no failure, push string into vector
		this->m_ramfile.push_back(lineStr);
	}
	
	//Close file for next operation
	resetAndClose();
	
	//If verbosity is enabled, print a nice message
	if(this->VERBOSE == true) {
		std::cout << "Read " << this->m_filename << " Successful: "
			<< this->bytes() << " bytes, " << this->lines() << " lines."
			<< std::endl;
	}
	
	//Success
	return EXIT_SUCCESS;
}

std::string TeFiEd::getLine(size_t index) {
	//Decriment index if above 0, vector is 0 indexed but 
	//line number is 1 indexed
	if(index > 0) {
		--index;
	}
	
	//TODO segfault somehow	
	if(index > this->m_ramfile.size() - 1) {
		errorMsg("getLine", "Line", index + 1, "does not exist");
		
		return "";
	}
	
	//If everything is normal
	return this->m_ramfile[index];
}

int TeFiEd::overwrite() {
	//Open file as output, truncate
	this->m_file.open(m_filename, std::ios::out | std::ios::trunc);
	
	//Make sure file is open and exists
	if(m_file.is_open() == 0) { 
		errorMsg("overwrite", "Could not create file");
		
		return EXIT_FAILURE;
	}
	
	//Write parent object ram to file
	for(std::string lineStr : this->m_ramfile) {
		m_file << lineStr << std::endl;
	}
	
	//Close file and clear flags
	resetAndClose();
	
	//If verbosity is enabled, print a nice message
	if(this->VERBOSE == true) {
		std::cout << "Overwrite " << this->m_filename << " Successful: wrote "
			<< this->bytes() << " bytes, " << this->lines() << " lines."
			<< std::endl;
	}
	
	return EXIT_SUCCESS;
}

//Write RAM into file object passed to it
int TeFiEd::writeTo(TeFiEd &target) {
	//Open file as output, truncate
	target.m_file.open(target.m_filename, std::ios::out | std::ios::trunc);
	
	//Make sure file is open and exists
	if(target.m_file.is_open() == 0) { 
		errorMsg("writeTo", "Could not create file ", target.m_filename);
		
		return EXIT_FAILURE;
	}
	
	//Write parent ram to reference file
	for(std::string lineStr : this->m_ramfile) {
		target.m_file << lineStr << std::endl;
	}
	
	//Close file and clear flags
	target.resetAndClose();
	
	//If verbosity is enabled, print a nice message
	if(this->VERBOSE == true) {
		std::cout << "Write to " << target.m_filename << " Successful: wrote "
			<< this->bytes() << " bytes, " << this->lines() << " lines."
			<< std::endl;
	}
	
	return EXIT_SUCCESS;
}

/** High Level Edit Functions **/
int TeFiEd::checkString(std::string testString, std::string callerFunct) {
	//Check input string for errors or problems
	
	size_t stringSize = testString.size();
	
	//Check input length
	if(stringSize > MAX_STRING_SIZE) {
		errorMsg(callerFunct, "input string exceeds MAX_STRING_SIZE -",
			MAX_STRING_SIZE);
		
		return EXIT_FAILURE;
	}
	
	//Check if adding the string to the file will overflow the ram limit
	//NOTE +1 for newline, because this is a line operation
	if((this->bytes() + stringSize + 1) > MAX_RAM_BYTES) {
		errorMsg(callerFunct, "Operation causes file to exceed MAX_RAM_BYTES -",
			MAX_RAM_BYTES);
		
		return EXIT_FAILURE;
	}
	
	//Otherwise exit with pass
	return EXIT_SUCCESS;
}

int TeFiEd::appendLine(const std::string text) {
	//Append string to end of RAM object
	
	//Sanity check string and RAM size
	if(checkString(text, "appendLine") == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	
	//push entry to back of the vector
	m_ramfile.push_back(text);
	//Complete
	return EXIT_SUCCESS;
}

int TeFiEd::insertLine(const std::string text, size_t index) {
	//Decriment index if above 0, vector is 0 indexed but 
	//line number is 1 indexed
	if(index > 0) {
		--index;
	}

	//Make sure that the vector has enough elements to allow the insert
	if(index > m_ramfile.size()) {
		//Error message and return fail
		errorMsg("insertLine", "Line", index + 1, "does not exist");
		
		return EXIT_FAILURE;
	}
	
	//Sanity check string and RAM size
	if(checkString(text, "insertLine") == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	
	m_ramfile.insert(m_ramfile.begin() + index, text);
	return EXIT_SUCCESS;
}

int TeFiEd::appendString(const std::string text, size_t index) {
	//Decriment index if above 0, vector is 0 indexed but 
	//line number is 1 indexed
	if(index > 0) {
		--index;
	}
	
	//Combine lengths of both input and pre-existing string for length check
	std::string catString = m_ramfile[index] + text;
	//Sanity check string and RAM size
	if(checkString(catString, "appendString") == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	
	//append the string in the vector at index given
	m_ramfile[index].append(text);
	
	return EXIT_SUCCESS;
}

int TeFiEd::insertString(const std::string text, size_t index, size_t pos) {
	//Insert a string at the given position	
	
	//Decriment index if above 0, vector is 0 indexed but 
	//line number is 1 indexed
	if(index > 0) --index;
	
	//Decriment pos if above 0
	if(pos > 0) --pos;
	
	//Make sure that the vector has enough elements to allow the insert
	if(index > m_ramfile.size()) {
		errorMsg("insertString", "Line", index + 1, "does not exist");
		
		return EXIT_FAILURE;
	}
	
	//Make sure that pos doesn't go past the string in vector[index]
	if(pos > m_ramfile[index].size()) {
		errorMsg("insertString", "cannot insert to line", index + 1,
			"at position " + std::to_string(pos + 1));
		
		return EXIT_FAILURE;
	}
	
	//Combine lengths of both input and pre-existing string for length check
	std::string catString = m_ramfile[index] + text;
	//Sanity check string and RAM size
	if(checkString(catString, "insertString") == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	
	//If nothing goes wrong, append to the string
	m_ramfile[index].insert(pos, text);
	return EXIT_SUCCESS;
}

int TeFiEd::removeLine(size_t index) {
	//Decriment index if above 0, vector is 0 indexed but 
	//line number is 1 indexed
	if(index > 0) {
		--index;
	}
	
	//Make sure that the vector has the correct number of elements
	if(index > m_ramfile.size()) {
		//Error message and return error value
		errorMsg("removeLine", "Line", index + 1, "does not exist");
			
		return EXIT_FAILURE;
	}
		
	//Erase line specified
	m_ramfile.erase(m_ramfile.begin() + index);
	//Shrink the vector
	m_ramfile.shrink_to_fit();
	
	//Return success
	return EXIT_SUCCESS;
}
