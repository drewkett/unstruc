#ifndef SU2_H_EC3A21BF_F313_487B_8B22_60E94691FC83
#define SU2_H_EC3A21BF_F313_487B_8B22_60E94691FC83

#include <string>

struct Grid;

bool su2_write(const std::string& filename, const Grid &grid);
Grid su2_read(const std::string& filename);

#endif
