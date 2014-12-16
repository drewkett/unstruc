#include "vtk.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>
#include <fstream>
#include <string>

bool toVTK(std::string &outputfile, Grid &grid) {
	std::fstream f;
	f.open(outputfile.c_str(),std::ios::out);
	if (!f.is_open()) Fatal("Could not open file");
	f.precision(15);
	Point * p;
	Element * e;
	set_i(grid);
	f << "# vtk DataFile Version 2.0" << std::endl;
	f << "Description" << std::endl;
	f << "ASCII" << std::endl;
	f << "DATASET UNSTRUCTURED_GRID" << std::endl;
	f << "POINTS " << grid.n_points << " double" << std::endl;
	for (int i = 0; i < grid.ppoints.size(); i++) {
		if (!grid.ppoints[i]) continue;
		p = *grid.ppoints[i];
		f << p->x << " " << p->y;
		if (grid.dim == 3)
			f << " " << p->z << std::endl;
		else
			f << " 0.0" << std::endl;
	}
	int n_elvals = 0;
	for (int i = 0; i < grid.elements.size(); i++) {
		Element &e = grid.elements[i];
		if (!e.valid) continue;
		if (e.dim != grid.dim) continue;
		n_elvals += e.len+1;
	}
	f << "CELLS " << grid.n_elems << " " << n_elvals << std::endl;
	for (int i = 0; i < grid.elements.size(); i++) {
		Element &e = grid.elements[i];
		if (!e.valid) continue;
		if (e.dim != grid.dim) continue;
		f << e.len;
		for (int j = 0; j < e.len; j++) {
			f << " " << (**e.points[j]).i;
		}
		f << std::endl;
	}
	f << "CELL_TYPES " << grid.n_elems << std::endl;
	for (int i = 0; i < grid.elements.size(); i++) {
		if (!grid.elements[i].valid) continue;
		Element &e = grid.elements[i];
		if (e.dim != grid.dim) continue;
		f << e.type << std::endl;
	}
	return true;
}
