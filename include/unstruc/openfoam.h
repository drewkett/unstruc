#ifndef OPENFOAM_H_11067D71_D27E_4CC7_BE3A_98143886996B
#define OPENFOAM_H_11067D71_D27E_4CC7_BE3A_98143886996B

#include <string>

namespace unstruc {
	struct Grid;
	Grid openfoam_read(const std::string& filename);
}

#endif
