#include <vector>

struct Element;
struct Point;

struct Name {
	int dim;
	int i;
	char name[20];
};

struct Grid {
	std::vector <Point *> points;
	std::vector <Point **> ppoints;
	std::vector <Element *> elements;
	std::vector <Name *> names;
	int n_points;
	int n_elems;
	int n_names;
};

bool set_i(Grid * grid);
