#include <vector>
#include <deque>

struct Element;
struct Point;

struct Name {
	int dim;
	int i;
	char name[20];
};

struct Grid {
	std::deque <Point *> points;
	std::deque <Point **> ppoints;
	std::deque <Element *> elements;
	std::deque <Name *> names;
	int n_points;
	int n_elems;
	int n_names;
};

bool set_i(Grid * grid);
void set_s_points(Grid * grid);
void set_s_elements(Grid * grid);
void merge_points(Grid * grid, double tol);
void delete_inner_faces(Grid * grid);
void collapse_elements(Grid * grid);
