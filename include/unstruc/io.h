#ifndef IO_H_0D62E640_6D46_4047_849D_C11CE35E91AE
#define IO_H_0D62E640_6D46_4047_849D_C11CE35E91AE

#include <string>

struct Grid;

enum FileType {
	UNKNOWN = 0,
	PLOT3D = 1,
	SU2 = 2,
	VTK = 3,
	OPENFOAM = 4,
	STL = 5,
	GMSH = 6,
};

FileType filetype_from_filename(std::string filename);
Grid read_grid(std::string filename);
void write_grid(std::string filename, Grid& grid);

#endif