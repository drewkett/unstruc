#include "farfield.h"

#include "unstruc.h"
#include "surface.h"

Grid create_farfield_box(Grid const& surface) {
	double min_x = 1e10;
	double min_y = 1e10;
	double min_z = 1e10;
	double max_x = -1e10;
	double max_y = -1e10;
	double max_z = -1e10;
	for (Point const& p : surface.points) {
		if (p.x < min_x) min_x = p.x;
		if (p.y < min_y) min_y = p.y;
		if (p.z < min_z) min_z = p.z;
		if (p.x > max_x) max_x = p.x;
		if (p.y > max_y) max_y = p.y;
		if (p.z > max_z) max_z = p.z;
	}
	double dx = max_x-min_x;
	double dy = max_y-min_y;
	double dz = max_z-min_z;
	double max_length = std::max(std::max(dx,dy),dz);
	double delta = 10*max_length;
	min_x -= delta;
	min_y -= delta;
	min_z -= delta;
	max_x += delta;
	max_y += delta;
	max_z += delta;
	Grid farfield (3);
	farfield.points.emplace_back(min_x, min_y, min_z);
	farfield.points.emplace_back(max_x, min_y, min_z);
	farfield.points.emplace_back(max_x, max_y, min_z);
	farfield.points.emplace_back(min_x, max_y, min_z);
	farfield.points.emplace_back(min_x, min_y, max_z);
	farfield.points.emplace_back(max_x, min_y, max_z);
	farfield.points.emplace_back(max_x, max_y, max_z);
	farfield.points.emplace_back(min_x, max_y, max_z);
	Element e1 (QUAD);
	e1.points = {0,1,2,3};
	farfield.elements.push_back(e1);
	Element e2 (QUAD);
	e2.points = {4,5,6,7};
	farfield.elements.push_back(e2);
	Element e3 (QUAD);
	e3.points = {0,1,5,4};
	farfield.elements.push_back(e3);
	Element e4 (QUAD);
	e4.points = {2,3,7,6};
	farfield.elements.push_back(e4);
	Element e5 (QUAD);
	e5.points = {1,2,6,5};
	farfield.elements.push_back(e5);
	Element e6 (QUAD);
	e6.points = {3,0,4,7};
	farfield.elements.push_back(e6);

	double max_area = 4*(max_length*max_length);
	Grid tetra_farfield = tetrahedralize_surface(farfield,max_area);
	return farfield;
}
