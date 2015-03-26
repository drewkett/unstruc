#include "vtk.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

namespace unstruc {

bool vtk_write(const std::string& filename, const Grid &grid) {
	std::cerr << "Writing " << filename << std::endl;
	std::ofstream f (filename);
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
	std::ofstream f (filename, std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f << std::endl << "POINT_DATA " << grid.points.size() << std::endl;
}

void vtk_write_cell_data_header(const std::string& filename, const Grid& grid) {
	std::ofstream f (filename, std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f << std::endl << "CELL_DATA " << grid.points.size() << std::endl;
}

void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <int>& scalars) {
	std::ofstream f (filename, std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f << "SCALARS " << name << " int 1" << std::endl;
	f << "LOOKUP_TABLE" << std::endl;
	for (int s : scalars) {
		f << s << std::endl;
	}
	f << std::endl;
}

void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <double>& scalars) {
	std::ofstream f (filename, std::ios::app);
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
	std::ofstream f (filename, std::ios::app);
	if (!f.is_open()) fatal("Could not open file");
	f.precision(15);
	f << "Vectors " << name << " double" << std::endl;
	for (const Vector& v : vectors) {
		f << v.x << " " << v.y << " " << v.z << std::endl;
	}
	f << std::endl;
}

std::unique_ptr<Grid> vtk_read_ascii(std::ifstream& f) {
	std::unique_ptr<Grid> grid (new Grid (3));
	std::string token;
	f >> token;
	if (token != "DATASET") return nullptr;

	f >> token;
	if (token != "UNSTRUCTURED_GRID") return nullptr;

	f >> token;
	if (token != "POINTS") return nullptr;

	int n_points;
	f >> n_points;
	if (!f) return nullptr;

	f >> token;
	if (token != "double") return nullptr;

	grid->points.reserve(n_points);
	for (int i = 0; i < n_points; ++i) {
		Point p;
		f >> p.x;
		f >> p.y;
		f >> p.z;
		grid->points.push_back(p);
	}
	if (!f) return nullptr;

	f >> token;
	if (token != "CELLS") return nullptr;

	int n_cells, n_cells_size;
	f >> n_cells;
	f >> n_cells_size;
	if (!f) return nullptr;

	std::vector<int> cells (n_cells_size);
	for (int i = 0; i < n_cells_size; ++i) {
		f >> cells[i];
	}

	f >> token;
	if (token != "CELL_TYPES") return nullptr;

	int n_cells2;
	f >> n_cells2;
	if (!f || n_cells != n_cells2) return nullptr;

	grid->elements.reserve(n_cells);

	int cj = 0;
	for (int i = 0; i < n_cells; ++i) {
		int vtk_id;
		f >> vtk_id;
		Shape::Type type = type_from_vtk_id(vtk_id);

		int n_elem_points = cells[cj];
		cj++;

		if (Shape::Info[type].n_points && Shape::Info[type].n_points != n_elem_points)
			return nullptr;

		Element e (type);
		if (Shape::Info[type].n_points == 0)
			e.points.resize(n_elem_points);

		for (int j = 0; j < n_elem_points; ++j) {
			e.points[j] = cells[cj];
			cj++;
		}
		grid->elements.push_back(e);
	}
	if (!f) return nullptr;
	if (cj != n_cells_size) return nullptr;

	return std::move(grid);
}

Grid vtk_read(const std::string& filename) {
	std::cerr << "Reading " << filename << std::endl;
	std::ifstream f (filename);
	if (!f.is_open()) fatal("Could not open file");

	char line[256];
	f.getline(line,256);
	if (strncmp(line,"# vtk DataFile Version 2.0",256) != 0)
		fatal("Invalid VTK File : " + filename);
	f.getline(line,256);
	std::string type;
	f >> type;
	std::unique_ptr<Grid> grid;
	if (type == "ASCII")
		grid = vtk_read_ascii(f);
	else if (type == "BINARY")
		not_implemented("Currently don't support binary vtk files");

	if (!grid)
		fatal("Error reading VTK file : "+filename);
	return *grid;
}

} // namespace unstruc
