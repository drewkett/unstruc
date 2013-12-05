#include "su2.h"

#include "grid.h"
#include "element.h"
#include "point.h"

#include <iostream>
#include <stdlib.h>

bool toSU2(Grid * grid) {
	std::cout.precision(15);
	Name * name;
	Point * p;
	Element * e;
	set_i(grid);
	sortElementsByName(grid);
	std::cout << "NDIME= " << 3 << std::endl;
	std::cout << std::endl;
	std::cout << "NELEM= " << grid->n_elems << std::endl;
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!is3D(grid->elements[i])) continue;
		e = grid->elements[i];
		std::cout << e->type;
		for (int j = 0; j < e->len; j++) {
			std::cout << " " << (**e->points[j]).i;
		}
		std::cout << " " << i;
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << "NPOIN= " << grid->n_points << std::endl;
	for (int i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		p = *grid->ppoints[i];
		std::cout << p->x << " " << p->y << " " << p->z << " " << p->i << std::endl;
	}
	std::cout << std::endl;
	int n_names = 0;
	for (int i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		if (grid->names[i]->dim != 2) continue;
		n_names++;
	}
	int* name_count = new int[grid->names.size()];
	for (int i = 0; i < grid->names.size(); i++) {
		name_count[i] = 0;
	}
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!is2D(grid->elements[i])) continue;
		e = grid->elements[i];
		name_count[e->name_i]++;
	}
	std::cout << "NMARK= " << n_names << std::endl;
	std::cerr << "MARKERS:" << std::endl;
	for (int i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		name = grid->names[i];
		if (name->dim != 2) continue;
		std::cerr << i << " : " << name->name << std::endl;
		std::cout << "MARKER_TAG= " << name->name << std::endl;
		std::cout << "MARKER_ELEMS= " << name_count[i] << std::endl;
		for (int j = 0; j < grid->elements.size(); j++) {
			if (!is2D(grid->elements[j])) continue;
			e = grid->elements[j];
			if (e->name_i != i) continue;
			std::cout << e->type;
			for (int k = 0; k < e->len; k++) {
				std::cout << " " << (**e->points[k]).i;
			}
			std::cout << std::endl;
		}
	}
	return true;
}
