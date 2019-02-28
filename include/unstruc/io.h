#ifndef IO_H_0D62E640_6D46_4047_849D_C11CE35E91AE
#define IO_H_0D62E640_6D46_4047_849D_C11CE35E91AE

#include <string>

namespace unstruc {
	struct Grid;

	enum struct FileType {
		Unknown,
		Plot3D,
		SU2,
		VTK,
		OpenFoam,
		STL,
		STLB,
		GMSH,
		CGNS2,
		Count
	};

	FileType filetype_from_filename(const std::string& filename);
	Grid read_grid(const std::string& filename);
	void write_grid(const std::string& filename,const Grid& grid);
}

#endif
