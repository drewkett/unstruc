#ifndef ERROR_H_DA99BDED_76EB_44AE_ADD3_4681CDD6A89A
#define ERROR_H_DA99BDED_76EB_44AE_ADD3_4681CDD6A89A

#include <string>

void fatal();
void fatal(std::string s);
void not_implemented();
void not_implemented(std::string s);
void error(std::string s);

#endif
