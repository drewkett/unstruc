#include "error.h"

#include <iostream>
#include <string>
#include <cstdlib>

void Fatal() {
	std::cerr << "Fatal Error" << std::endl;
	exit(1);
}

void Fatal(std::string s) {
	std::cerr << s << std::endl << "Fatal Error" << std::endl;
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
