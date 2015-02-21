#ifndef ERROR_H_DA99BDED_76EB_44AE_ADD3_4681CDD6A89A
#define ERROR_H_DA99BDED_76EB_44AE_ADD3_4681CDD6A89A

#include <string>

void Fatal();
void Fatal(std::string s);
void NotImplemented();
void NotImplemented(std::string s);
void Error(std::string s);

#endif
