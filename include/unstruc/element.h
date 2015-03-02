#ifndef ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42
#define ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42

#include <vector>
#include <string>

struct Point;

struct Shape {
	std::string name;
	int dim, n_points, vtk_id;

	static const Shape Info[];

	enum Type {
		Undefined,
		Line,
		Triangle,
		Quad,
		Polygon,
		Tetra,
		Hexa,
		Wedge,
		Pyramid,
		NShapes
	};
};

Shape::Type type_from_vtk_id(int vtk_id);

struct Grid;

struct Element
{
	Shape::Type type;
	int name_i;
	std::vector<int> points;

	Element() : type(Shape::Type::Undefined), name_i(0) {};
	Element(Shape::Type T);

	double calc_volume(Grid& grid);

};

void dump(Element &e);
void dump(Element &e, Grid& grid);
bool same(Element &e1, Element &e2);

bool can_collapse(Element& e);
bool collapse(Element& e,std::vector<Element>& new_elements);

bool can_collapse_wo_split(Element& e);
bool collapse_wo_split(Element& e);

#endif
