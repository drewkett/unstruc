#ifndef GMSH_H_04714453_D3B2_49A0_8973_EE6840FEAF5C
#define GMSH_H_04714453_D3B2_49A0_8973_EE6840FEAF5C

#include <string> 

namespace unstruc {
	struct Grid;

	void gmsh_write(const std::string& filename, const Grid &grid);
}

#endif
