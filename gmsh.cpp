#include "gmsh.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>

bool toGMSH(Grid &grid) {
	std::cout.precision(15);
	Name name;
	//set_i(grid);
	std::cout << "$MeshFormat" << std::endl;
	std::cout << "2.2 0 " << sizeof(double) << std::endl;
	std::cout << "$EndMeshFormat" << std::endl;
	std::cout << "$Nodes" << std::endl;
	std::cout << grid.points.size() << std::endl;
	for (int i = 0; i < grid.points.size(); i++) {
		Point& p = grid.points[i];
		std::cout << i+1 << " " << p.x << " " << p.y << " " << p.z << std::endl;
	}
	std::cout << "$EndNodes" << std::endl;
	std::cout << "$Elements" << std::endl;
	std::cout << grid.elements.size() << std::endl;
	for (int i = 0; i < grid.elements.size(); i++) {
		Element &e = grid.elements[i];
		std::cout << i+1;
		switch (e.type) {
			case QUAD:
				std::cout << " 3";
				break;
			case HEXA:
				std::cout << " 5";
				break;
			default:
				NotImplemented("ElType for GMSH");
		}
		std::cout << " " << 2 << " " << e.name_i+1 << " " << e.name_i+1;
		for (int p : e.points) {
			std::cout << " " << p;
		}
		std::cout << std::endl;
	}
	std::cout << "$EndElements" << std::endl;
	std::cout << "$PhysicalNames" << std::endl;
	std::cout << grid.names.size() << std::endl;
	for (int i = 0; i < grid.names.size(); i++) {
		name = grid.names[i];
		std::cout << name.dim << " " << i+1 << " \"" << name.name << "\"" << std::endl;
	}
	std::cout << "$EndPhysicalNames" << std::endl;
	return true;
}
