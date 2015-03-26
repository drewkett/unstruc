
#include "io.h"

#include "plot3d.h"
#include "su2.h"
#include "gmsh.h"
#include "stl.h"
#include "vtk.h"

#include "grid.h"
#include "error.h"

namespace unstruc {

FileType filetype_from_filename(const std::string& filename) {
	int n = filename.size();
	if (n > 4 && filename.compare(n-4,4,".su2") == 0)
		return FileType::SU2;
	else if (n > 4 && filename.compare(n-4,4,".stl") == 0)
		return FileType::STL;
	else if (n > 5 && filename.compare(n-5,5,".stlb") == 0)
		return FileType::STLB;
	else if (n > 4 && filename.compare(n-4,4,".vtk") == 0)
		return FileType::VTK;
	else if (n > 4 && (filename.compare(n-4,4,".xyz") == 0 || filename.compare(n-4,4,".p3d") == 0))
		return FileType::Plot3D;
	else if ((n > 8 && filename.compare(n-8,8,"polyMesh") == 0) || (n > 9 && filename.compare(n-9,9,"polyMesh/") == 0))
		return FileType::OpenFoam;
	else
		fatal("Unknown filetype");
	return FileType::Unknown;
}

Grid read_grid(const std::string& filename) {
	FileType type = filetype_from_filename(filename);

	Grid grid;
	switch (type) {
		case FileType::Plot3D:
			return plot3d_read(filename);
		case FileType::SU2:
			return su2_read(filename);
		case FileType::STL:
			return stl_read(filename);
		case FileType::STLB:
			return stl_read_binary(filename);
		case FileType::VTK:
			return vtk_read(filename);
		default:
			fatal("Unsupported filetype for reading");
	}
	return grid;
}

void write_grid(const std::string& filename,const Grid& grid) {
	if (!grid.check_integrity())
		fatal("Grid integrity check failed");

	FileType type = filetype_from_filename(filename);

	switch (type) {
		case FileType::SU2:
			su2_write(filename,grid);
			break;
		case FileType::VTK:
			vtk_write(filename,grid);
			break;
		case FileType::GMSH:
			gmsh_write(filename,grid);
			break;
		case FileType::STL:
			stl_write_ascii(filename,grid);
			break;
		case FileType::STLB:
			stl_write_binary(filename,grid);
			break;
		default:
			fatal("Unsupported filetype for writing");
	}
}

} //namespace unstruc
