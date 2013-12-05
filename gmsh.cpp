#include "gmsh.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>

bool toGMSH(Grid * grid) {
	std::cout.precision(15);
	Point * p;
	Element * e;
	Name * name;
	set_i(grid);
	std::cout << "$MeshFormat" << std::endl;
	std::cout << "2.2 0 " << sizeof(double) << std::endl;
	std::cout << "$EndMeshFormat" << std::endl;
	std::cout << "$Nodes" << std::endl;
	std::cout << grid->n_points << std::endl;
	for (int i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		p = *grid->ppoints[i];
		std::cout << p->i+1 << " " << p->x << " " << p->y << " " << p->z << std::endl;
	}
	std::cout << "$EndNodes" << std::endl;
	std::cout << "$Elements" << std::endl;
	std::cout << grid->n_elems << std::endl;
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!grid->elements[i]) continue;
		e = grid->elements[i];
		std::cout << e->i+1;
		switch (e->type) {
			case QUAD:
				std::cout << " 3";
				break;
			case HEXA:
				std::cout << " 5";
				break;
			default:
				NotImplemented("ElType for GMSH");
		}
		std::cout << " " << 2 << " " << e->name_i+1 << " " << e->name_i+1;
		for (int j = 0; j < e->len; j++) {
			std::cout << " " << (**e->points[j]).i+1;
		}
		std::cout << std::endl;
	}
	std::cout << "$EndElements" << std::endl;
	std::cout << "$PhysicalNames" << std::endl;
	std::cout << grid->names.size() << std::endl;
	for (int i = 0; i < grid->names.size(); i++) {
		name = grid->names[i];
		std::cout << name->dim << " " << i+1 << " \"" << name->name << "\"" << std::endl;
	}
	std::cout << "$EndPhysicalNames" << std::endl;
	return true;
}
