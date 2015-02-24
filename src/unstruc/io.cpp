
#include "io.h"

#include "plot3d.h"
#include "su2.h"
#include "gmsh.h"
#include "stl.h"
#include "vtk.h"

#include "grid.h"
#include "error.h"

FileType filetype_from_filename(const std::string& filename) {
	int n = filename.size();
	if (filename.compare(n-4,4,".su2") == 0)
		return FileType::SU2;
	else if (filename.compare(n-4,4,".stl") == 0)
		return FileType::STL;
	else if (filename.compare(n-4,4,".vtk") == 0)
		return FileType::VTK;
	else if (filename.compare(n-4,4,".xyz") == 0 || filename.compare(n-4,4,".p3d") == 0)
		return FileType::Plot3D;
	else if (filename.compare(n-8,8,"polyMesh") == 0 || filename.compare(n-9,9,"polyMesh/") == 0)
		return FileType::OpenFoam;
	else
		Fatal("Unknown filetype");
	return FileType::Unknown;
}

Grid read_grid(const std::string& filename) {
	FileType type = filetype_from_filename(filename);

	Grid grid;
	switch (type) {
		case FileType::Plot3D:
			return readPlot3D(filename);
		case FileType::SU2:
			return readSU2(filename);
		case FileType::STL:
			return readSTL(filename);
		default:
			Fatal("Unsupported filetype for reading");
	}
	return grid;
}

void write_grid(const std::string& filename,Grid& grid) {
	if (!grid.check_integrity())
		Fatal("Grid integrity check failed");

	FileType type = filetype_from_filename(filename);

	switch (type) {
		case FileType::SU2:
			toSU2(filename,grid);
			break;
		case FileType::VTK:
			toVTK(filename,grid);
			break;
		case FileType::GMSH:
			toGMSH(filename,grid);
			break;
		default:
			Fatal("Unsupported filetype for writing");
	}
}
