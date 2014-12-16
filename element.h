#ifndef ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42
#define ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42

#include "point.h"

#include <vector>

enum Shapes { LINE = 3,
			  TRI = 5,
			  QUAD = 9,
			  TETRA = 10,
			  HEXA = 12,
			  WEDGE = 13,
			  PYRAMID = 14};

struct Element
{
	int type;
	int len;
	int name_i;
	int i;
	int dim;
	Point * s;
	std::vector<Point **> points;
	bool valid;
	Element() : valid(false) {};
	Element(int T);
};

void dump(Element * e);
void set_s_by_lowest_id(Element &e);
bool compare_element(Element &e1, Element &e2);
bool compare_element_by_name(Element &e1, Element &e2);
bool compare_element_by_index(Element &e1, Element &e2);
bool close(Element &e1, Element &e2);
bool same(Element &e1, Element &e2);

Element * tri_from_quad(Element * e,int i1, int i2, int i3);

Element * pyramid_from_hexa(Element * e,int i1, int i2, int i3, int i4, int i5);
Element * wedge_from_hexa(Element * e,int i1, int i2, int i3, int i4, int i5, int i6);

bool canCollapse(Element * e);

Element * collapse_tri(Element * e);
Element * collapse_quad(Element * e);
Element * collapse_pyramid(Element * e);
Element * collapse_wedge(Element * e) ;
Element * collapse_hexa(Element * e);
Element * collapse(Element * e);

#endif
