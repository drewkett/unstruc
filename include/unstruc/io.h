#ifndef IO_H_0D62E640_6D46_4047_849D_C11CE35E91AE
#define IO_H_0D62E640_6D46_4047_849D_C11CE35E91AE

#include <string>

struct Grid;

enum struct FileType {
	Unknown,
	Plot3D,
	SU2,
	VTK,
	OpenFoam,
	STL,
	GMSH,
	Count
};

FileType filetype_from_filename(std::string filename);
Grid read_grid(std::string filename);
void write_grid(std::string filename, Grid& grid);

#endif
