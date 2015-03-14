#include "farfield.h"

#include "unstruc.h"
#include "surface.h"

using namespace unstruc;

namespace tetmesh {

Grid create_farfield_box(Grid const& surface) {
	Point min = surface.get_bounding_min();
	Point max = surface.get_bounding_max();
	Vector d = max - min;
	double max_length = std::max(std::max(d.x,d.y),d.z);
	Vector delta { 1, 1, 1 };
	delta *= 10*max_length;
	min -= delta;
	max += delta;
	Grid farfield (3);
	farfield.points.push_back( Point {min.x, min.y, min.z} );
	farfield.points.push_back( Point {max.x, min.y, min.z} );
	farfield.points.push_back( Point {max.x, max.y, min.z} );
	farfield.points.push_back( Point {min.x, max.y, min.z} );
	farfield.points.push_back( Point {min.x, min.y, max.z} );
	farfield.points.push_back( Point {max.x, min.y, max.z} );
	farfield.points.push_back( Point {max.x, max.y, max.z} );
	farfield.points.push_back( Point {min.x, max.y, max.z} );
	Element e1 (Shape::Quad);
	e1.points = {0,1,2,3};
	farfield.elements.push_back(e1);
	Element e2 (Shape::Quad);
	e2.points = {4,5,6,7};
	farfield.elements.push_back(e2);
	Element e3 (Shape::Quad);
	e3.points = {0,1,5,4};
	farfield.elements.push_back(e3);
	Element e4 (Shape::Quad);
	e4.points = {2,3,7,6};
	farfield.elements.push_back(e4);
	Element e5 (Shape::Quad);
	e5.points = {1,2,6,5};
	farfield.elements.push_back(e5);
	Element e6 (Shape::Quad);
	e6.points = {3,0,4,7};
	farfield.elements.push_back(e6);

	double max_area = 4*(max_length*max_length);
	Grid tetra_farfield = tetrahedralize_surface(farfield,max_area);
	tetra_farfield.names[0].name = "farfield";
	return tetra_farfield;
}

}
