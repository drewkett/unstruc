#ifndef GMSH_H_04714453_D3B2_49A0_8973_EE6840FEAF5C
#define GMSH_H_04714453_D3B2_49A0_8973_EE6840FEAF5C

#include <string> 

struct Grid;

void toGMSH(std::string filename, Grid &grid);

#endif
