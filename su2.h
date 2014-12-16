#ifndef SU2_H_EC3A21BF_F313_487B_8B22_60E94691FC83
#define SU2_H_EC3A21BF_F313_487B_8B22_60E94691FC83

#include <string>

struct Grid;

bool toSU2(std::string &outputfile, Grid &grid);
Grid * readSU2(std::string &inputfile);

#endif