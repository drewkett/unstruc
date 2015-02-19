
#include "io.h"

#include "plot3d.h"
#include "su2.h"
#include "gmsh.h"
#include "stl.h"
#include "vtk.h"

#include "grid.h"
#include "error.h"

FileType filetype_from_filename(std::string filename) {
	int n = filename.size();
	if (filename.compare(n-4,4,".su2") == 0)
		return SU2;
	else if (filename.compare(n-4,4,".stl") == 0)
		return STL;
	else if (filename.compare(n-4,4,".vtk") == 0)
		return VTK;
	else if (filename.compare(n-4,4,".xyz") == 0 || filename.compare(n-4,4,".p3d") == 0)
		return PLOT3D;
	else if (filename.compare(n-8,8,"polyMesh") == 0 || filename.compare(n-9,9,"polyMesh/") == 0)
		return OPENFOAM;
	else
		Fatal("Unknown filetype");
}

Grid read_grid(std::string filename) {
	FileType type = filetype_from_filename(filename);

	Grid grid;
	switch (type) {
		case PLOT3D:
			return readPlot3D(filename);
		case SU2:
			return readSU2(filename);
		case STL:
			return readSTL(filename);
		default:
			Fatal("Unsupported filetype for reading");
	}
	return grid;
}

void write_grid(Grid& grid, std::string filename) {
	FileType type = filetype_from_filename(filename);

	switch (type) {
		case SU2:
			toSU2(filename,grid);
			break;
		case VTK:
			toVTK(filename,grid);
			break;
		case GMSH:
			toGMSH(filename,grid);
			break;
		default:
			Fatal("Unsupported filetype for writing");
	}
}
