#include "vtk.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>
#include <fstream>
#include <string>

namespace unstruc {

bool vtk_write(const std::string& outputfile, const Grid &grid) {
	std::cerr << "Writing " << outputfile << std::endl;
	std::fstream f;
	f.open(outputfile.c_str(),std::ios::out);
	if (!f.is_open()) fatal("Could not open file");
	f.precision(15);
	f << "# vtk DataFile Version 2.0" << std::endl;
	f << "Description" << std::endl;
	f << "ASCII" << std::endl;
	f << "DATASET UNSTRUCTURED_GRID" << std::endl;
	//std::cerr << "Writing Points" << std::endl;
	f << "POINTS " << grid.points.size() << " double" << std::endl;
	for (const Point& p : grid.points) {
		f << p.x << " " << p.y;
		if (grid.dim == 3)
			f << " " << p.z << std::endl;
		else
			f << " 0.0" << std::endl;
	}
	//std::cerr << "Writing Cells" << std::endl;
	int n_volume_elements = 0;
	int n_elvals = 0;
	for (const Element& e : grid.elements) {
		n_volume_elements++;
		n_elvals += e.points.size()+1;
	}
	f << "CELLS " << n_volume_elements << " " << n_elvals << std::endl;
	for (const Element& e : grid.elements) {
		f << e.points.size();
		for (int p : e.points) {
			f << " " << p;
		}
		f << std::endl;
	}

	f << "CELL_TYPES " << n_volume_elements << std::endl;
	for (const Element& e : grid.elements) {
		f << Shape::Info[e.type].vtk_id << std::endl;
	}
	return true;
}

void vtk_write_point_data_header(const std::string& filename, const Grid& grid) {
	std::fstream f;
	f.open(filename.c_str(),std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f << std::endl << "POINT_DATA " << grid.points.size() << std::endl;
}

void vtk_write_cell_data_header(const std::string& filename, const Grid& grid) {
	std::fstream f;
	f.open(filename.c_str(),std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f << std::endl << "CELL_DATA " << grid.points.size() << std::endl;
}

void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <int>& scalars) {
	std::fstream f;
	f.open(filename.c_str(),std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f << "SCALARS " << name << " int 1" << std::endl;
	f << "LOOKUP_TABLE" << std::endl;
	for (int s : scalars) {
		f << s << std::endl;
	}
	f << std::endl;
}

void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <double>& scalars) {
	std::fstream f;
	f.open(filename.c_str(),std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f.precision(15);
	f << "SCALARS " << name << " double 1" << std::endl;
	f << "LOOKUP_TABLE default" << std::endl;
	for (double s : scalars) {
		f << s << std::endl;
	}
	f << std::endl;
}

void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <Vector>& vectors) {
	std::fstream f;
	f.open(filename.c_str(),std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f.precision(15);
	f << "Vectors " << name << " double" << std::endl;
	for (const Vector& v : vectors) {
		f << v.x << " " << v.y << " " << v.z << std::endl;
	}
	f << std::endl;
}

} // namespace unstruc
