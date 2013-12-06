#include <vector>
#include <deque>
#include <string>

struct Element;
struct Point;

struct Name {
	int dim;
	int i;
	std::string name;
};

struct Grid {
	std::deque <Point *> points;
	std::vector <Point **> ppoints;
	std::vector <Element *> elements;
	std::vector <Name *> names;
	int n_points;
	int n_elems;
	int n_names;
};

bool set_i(Grid * grid);
void sort_points(Grid * grid);
void merge_points(Grid * grid, double tol);
void merge_points_test(Grid * grid, double tol);
void delete_inner_faces(Grid * grid);
void collapse_elements(Grid * grid);
void sort_elements(Grid * grid);
void sort_elements_by_name(Grid * grid);
