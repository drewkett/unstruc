#ifndef ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42
#define ELEMENT_H_DFA32621_EEB4_401F_8C3B_FA778CAD6F42

#include <vector>
#include <string>

namespace unstruc {

	struct Point;
	struct Grid;

	struct Shape {
		std::string name;
		size_t dim, n_points, n_edges, n_faces, vtk_id;

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

	Shape::Type type_from_vtk_id(size_t vtk_id);


	struct Element
	{
		Shape::Type type;
		int name_i;
		std::vector<size_t> points;

		Element() : type(Shape::Type::Undefined), name_i(0) {};
		Element(Shape::Type T);

		double calc_volume(const Grid& grid) const;
	};

	void dump(const Element &e);
	void dump(const Element &e, const Grid& grid);
	bool same(Element e1, Element e2);

	bool can_collapse(const Element& e);
	bool collapse(Element& e,std::vector<Element>& new_elements);

	bool can_collapse_wo_split(const Element& e);
	bool collapse_wo_split(Element& e);

}

#endif
