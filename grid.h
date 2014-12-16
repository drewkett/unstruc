#include <vector>
#include <deque>
#include <string>

struct Element;
struct Point;

struct Name {
	int dim;
	int i;
	std::string name;
	bool deleted;
	Name() : deleted(false) {};
};

struct Grid {
	std::deque <Point *> points;
	std::vector <Point **> ppoints;
	std::vector <Element *> elements;
	std::vector <Name> names;
	int n_points;
	int n_elems;
	int n_boundelems;
	int dim;
	int n_names;
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
