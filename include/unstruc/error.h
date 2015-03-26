#ifndef ERROR_H_DA99BDED_76EB_44AE_ADD3_4681CDD6A89A
#define ERROR_H_DA99BDED_76EB_44AE_ADD3_4681CDD6A89A

#include <string>

namespace unstruc {
	void fatal();
	void fatal(const std::string& s);
	void not_implemented();
	void not_implemented(const std::string& s);
}

#endif
