#include "vtk.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>
#include <fstream>
#include <string>

bool toVTK(std::string outputfile, Grid &grid, bool include_all_elements) {
	std::cerr << "Writing '" << outputfile << "'" << std::endl;
	std::fstream f;
	f.open(outputfile.c_str(),std::ios::out);
	if (!f.is_open()) Fatal("Could not open file");
	f.precision(15);
	f << "# vtk DataFile Version 2.0" << std::endl;
	f << "Description" << std::endl;
	f << "ASCII" << std::endl;
	f << "DATASET UNSTRUCTURED_GRID" << std::endl;
	std::cerr << "Writing Points" << std::endl;
	f << "POINTS " << grid.points.size() << " double" << std::endl;
	for (Point& p : grid.points) {
		f << p.x << " " << p.y;
		if (grid.dim == 3)
			f << " " << p.z << std::endl;
		else
			f << " 0.0" << std::endl;
	}
	std::cerr << "Writing Cells" << std::endl;
	int n_volume_elements = 0;
	int n_elvals = 0;
	for (Element& e : grid.elements) {
		if (e.dim != grid.dim && !include_all_elements) continue;
		n_volume_elements++;
		n_elvals += e.points.size()+1;
	}
	f << "CELLS " << n_volume_elements << " " << n_elvals << std::endl;
	for (Element& e : grid.elements) {
		if (e.dim != grid.dim && !include_all_elements) continue;
		f << e.points.size();
		for (int p : e.points) {
			f << " " << p;
		}
		f << std::endl;
	}

	f << "CELL_TYPES " << n_volume_elements << std::endl;
	for (Element& e : grid.elements) {
		if (e.dim != grid.dim && !include_all_elements) continue;
		f << e.type << std::endl;
	}
	return true;
}
