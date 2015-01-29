#ifndef GRID_H_A744617F_958E_4AD8_88D3_F2C799D3A0BE
#define GRID_H_A744617F_958E_4AD8_88D3_F2C799D3A0BE

#include "element.h"

#include <vector>
#include <deque>
#include <string>

struct Point;

struct Name {
	int dim;
	int i;
	std::string name;
	bool deleted;
	Name() : deleted(false) {};
	Name(int dim,std::string name) : dim(dim), name(name), deleted(false) {};
};

struct Grid {
	std::deque <Point *> points;
	std::vector <Point **> ppoints;
	std::vector <Element> elements;
	std::vector <Name> names;
	int n_points;
	int n_elems;
	int n_boundelems;
	int dim;
	int n_names;

	Grid () : points(0), ppoints(0), elements(0), names(0), n_points(0), n_elems(0), n_boundelems(0), dim(0), n_names(0) {};
};

void set_i(Grid &grid);
void set_i_points(Grid &grid);
void set_i_elements(Grid &grid);
void sort_points_by_location(Grid &grid);
void sort_points_by_index(Grid &grid);
void merge_points(Grid &grid, double tol);
void delete_inner_faces(Grid &grid);
void collapse_elements(Grid &grid);
void sort_elements(Grid &grid);
void sort_elements_by_name(Grid &grid);
void sort_elements_by_index(Grid &grid);

#endif
