#ifndef GRID_H_A744617F_958E_4AD8_88D3_F2C799D3A0BE
#define GRID_H_A744617F_958E_4AD8_88D3_F2C799D3A0BE

#include "element.h"

#include <vector>
#include <string>

struct Point;

struct Name {
	int dim;
	std::string name;
	Name() : dim(0) {};
	Name(int dim,std::string name) : dim(dim), name(name) {};
};

struct Grid {
	std::vector <Point> points;
	std::vector <Element> elements;
	std::vector <Name> names;
	int dim;

	Grid () : dim(0);
	Grid (int _dim);
	void merge_points(double tol);
	void delete_inner_faces();
	void collapse_elements();
	Grid grid_from_elements(std::vector<Element>&);
	void add_grid(Grid&);
	void delete_empty_names();
};


#endif
