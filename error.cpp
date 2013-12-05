#include "error.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

void Fatal() {
	std::cerr << "Fatal Error" << std::endl;
	exit(1);
}

void Fatal(std::string s) {
	std::cerr << "Fatal Error" << std::endl << s << std::endl;
	exit(1);
}

void NotImplemented() {
	std::cerr << "Not Implemented Yet" << std::endl;
	exit(1);
}

void NotImplemented(std::string s) {
	std::cerr << "Not Implemented Yet : " << s << std::endl;
	exit(1);
}
