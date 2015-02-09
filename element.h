#ifndef ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42
#define ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42

#include "point.h"

#include <vector>

enum Shapes { LINE = 3,
			  TRI = 5,
			  POLYGON = 7,
			  QUAD = 9,
			  TETRA = 10,
			  HEXA = 12,
			  WEDGE = 13,
			  PYRAMID = 14};

struct Grid;

struct Element
{
	int type;
	int name_i;
	int dim;
	std::vector<int> points;

	Element() : type(0), name_i(0), dim(0) {};
	Element(int T);

	double calc_volume(Grid& grid);
};

void dump(Element &e, Grid& grid);
bool same(Element &e1, Element &e2);
bool can_collapse(Element& e);
bool collapse(Element& e,std::vector<Element>& new_elements);

#endif
