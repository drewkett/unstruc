#include "grid.h"
#include "element.h"
#include "point.h"

#include <iostream>

void set_s_points(Grid * grid) {
	for (int i = 0; i < grid->points.size(); i++) {
		set_s(grid->points[i]);
	}
}

void set_s_elements(Grid * grid) {
	for (int i = 0; i < grid->elements.size(); i++) {
		set_s(grid->elements[i]);
	}
}

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

void merge_points(Grid * grid, double tol) {
	int n = 0;
	for (int i = 0; i < grid->ppoints.size()-1; i++) {
		if (!grid->ppoints[i]) continue;
		for (int j = i+1; j < grid->ppoints.size(); j++) {
			if (!grid->ppoints[j]) continue;
			if (!close(*grid->ppoints[i],*grid->ppoints[j],tol)) break;
			if (same(*grid->ppoints[i],*grid->ppoints[j],tol)) {
				*grid->ppoints[j] = *grid->ppoints[i];
				grid->ppoints[j] = NULL;
				n++;
			}
		}
	}
	std::cerr << n << " Points Merged" << std::endl;
}

void delete_inner_faces(Grid * grid) {
	int n = 0;
	for (int i = 0; i < grid->elements.size() - 1; i++) {
		if (!is2D(grid->elements[i])) continue;
		bool kill = false;
		for (int j = i+1; j < grid->elements.size(); j++) {
			if (!is2D(grid->elements[j])) continue;
		    if (!close(grid->elements[i],grid->elements[j])) break;
			if (same(grid->elements[i],grid->elements[j])) {
				delete grid->elements[j];
				grid->elements[j] = NULL;
				kill = true;
				n++;
			}
		}
		if (kill) {
			kill = false;
			n++;
			delete grid->elements[i];
			grid->elements[i] = NULL;
		}
	}
	std::cerr << n << " Faces Deleted" << std::endl;
}

void collapse_elements(Grid * grid) {
	bool collapsed;
	int n = 0, n2 = 0;
	Element *e, *e_new;
	for (int i = 0; i < grid->elements.size(); i++) {
		e = grid->elements[i];
		if (!e) continue;
		if (canCollapse(e)) {
			e_new = collapse(e);
			n++;
			if (e_new) {
				grid->elements.push_back(e_new);
				n2++;
			}
		}
	}
	std::cerr << n << " Elements Collapsed" << std::endl;
	std::cerr << n2 << " Elements Created on Collapse" << std::endl;
};
