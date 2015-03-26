#include "error.h"

#include <iostream>
#include <string>
#include <cstdlib>

namespace unstruc {

void fatal() {
	std::cerr << "Fatal Error" << std::endl;
	exit(1);
}

void fatal(const std::string& s) {
	std::cerr << s << std::endl << "Fatal Error" << std::endl;
	exit(1);
}

void not_implemented() {
	std::cerr << "Not Implemented Yet" << std::endl;
	exit(1);
}

void not_implemented(const std::string& s) {
	std::cerr << "Not Implemented Yet : " << s << std::endl;
	exit(1);
}

} //namespace unstruc
