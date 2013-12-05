#include "grid.h"
#include "element.h"
#include "point.h"

bool set_i(Grid * grid) {
	int i_p = 0;
	for (int i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		(*grid->ppoints[i])->i = i_p;
		i_p++;
	}
	grid->n_points = i_p;
	bool* name_mask = new bool[grid->names.size()];
	for (int i = 0; i < grid->names.size(); i++) {
		name_mask[i] = false;
	}
	int i_e = 0;
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!grid->elements[i]) continue;
		name_mask[grid->elements[i]->name_i] = true;
		if (!is3D(grid->elements[i])) continue;
		grid->elements[i]->i = i_e;
		i_e++;
	}
	grid->n_elems = i_e;
	int i_n = 0;
	for (int i = 0; i < grid->names.size(); i++) {
		if (name_mask[i] && grid->names[i]->dim == 2) {
			grid->names[i]->i = i_n;
			i_n++;
		} else {
			delete grid->names[i];
			grid->names[i] = NULL;
		}
	}
	grid->n_names = i_n;
	return true;
}
