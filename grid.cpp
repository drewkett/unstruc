#include "grid.h"
#include "element.h"
#include "point.h"

#include <iostream>
#include <algorithm>

void set_i_points(Grid &grid) {
	int i_p = 0;
	for (int i = 0; i < grid.ppoints.size(); i++) {
		if (!grid.ppoints[i]) continue;
		(*grid.ppoints[i])->i = i_p;
		i_p++;
	}
	grid.n_points = i_p;
}

void set_i_elements(Grid &grid) {
	int i_e = 0;
	for (int i = 0; i < grid.elements.size(); i++) {
		if (!grid.elements[i]) continue;
		grid.elements[i]->i = i_e;
		i_e++;
	}
	grid.n_elems = i_e;
}

void set_i(Grid &grid) {
	set_i_points(grid);
	std::vector <bool> name_mask(grid.names.size());
	for (int i = 0; i < grid.names.size(); i++) {
		name_mask[i] = false;
	}
	int i_e = 0, i_eb = 0;
	Element * e;
	for (int i = 0; i < grid.elements.size(); i++) {
		e = grid.elements[i];
		if (!e) continue;
		if (e->name_i < 0) continue;
		name_mask[e->name_i] = true;
		switch (grid.dim - e->dim) {
			case 0:
				i_e++;
				e->i = i_e;
				break;
			case 1:
				i_eb++;
				e->i = i_eb;
				break;
		}
	}
	grid.n_elems = i_e;
	grid.n_boundelems = i_eb;
}


void sort_points_by_index(Grid &grid){
	std::cerr << "Sorting Points By Index" << std::endl;
	sort(grid.ppoints.begin(),grid.ppoints.end(),compare_ppoint_by_index);
};

void sort_points_by_location(Grid &grid){
	Point * p;
	std::cerr << "Sorting Points" << std::endl;
	for (int i = 0; i < grid.points.size(); i++) {
		p = grid.points[i];
		p->s = p->x + p->y + p->z;
	}
	sort(grid.ppoints.begin(),grid.ppoints.end(),compare_ppoint);
};

void merge_points(Grid &grid, double tol) {
	std::cerr << "Merging Points" << std::endl;
	int n = 0;
	for (int i = 0; i < grid.ppoints.size()-1; i++) {
		if (!grid.ppoints[i]) continue;
		for (int j = i+1; j < grid.ppoints.size(); j++) {
			if (!grid.ppoints[j]) continue;
			if (!close(**grid.ppoints[i],**grid.ppoints[j],tol)) break;
			if (same(**grid.ppoints[i],**grid.ppoints[j],tol)) {
				*grid.ppoints[j] = *grid.ppoints[i];
				grid.ppoints[j] = NULL;
				n++;
			}
		}
	}
	std::cerr << n << " Points Merged" << std::endl;
}

void delete_inner_faces(Grid &grid) {
	int n = 0;
	Element *ei,*ej;
	for (int i = 0; i < grid.elements.size() - 1; i++) {
		ei = grid.elements[i];
		if (!ei or ei->dim != 2) continue;
		bool kill = false;
		for (int j = i+1; j < grid.elements.size(); j++) {
			ej = grid.elements[j];
			if (!ej or ej->dim != 2) continue;
		    if (!close(ei,ej)) break;
			if (same(ei,ej)) {
				delete ej;
				grid.elements[j] = NULL;
				kill = true;
				n++;
			}
		}
		if (kill) {
			kill = false;
			n++;
			delete ei;
			grid.elements[i] = NULL;
		}
	}
	std::cerr << n << " Faces Deleted" << std::endl;
}

void collapse_elements(Grid &grid) {
	bool collapsed;
	int n = 0, n2 = 0;
	Element *e, *e_new;
	for (int i = 0; i < grid.elements.size(); i++) {
		e = grid.elements[i];
		if (!e) continue;
		if (canCollapse(e)) {
			e_new = collapse(e);
			n++;
			if (e_new) {
				grid.elements.push_back(e_new);
				n2++;
			}
		}
	}
	std::cerr << n << " Elements Collapsed" << std::endl;
	std::cerr << n2 << " Elements Created on Collapse" << std::endl;
};

void sort_elements(Grid &grid){
	std::cerr << "Sorting Elements" << std::endl;
	for (int i = 0; i < grid.elements.size(); i++) {
		set_s_by_lowest_id(grid.elements[i]);
	}
	sort(grid.elements.begin(),grid.elements.end(),compare_element);
};

void sort_elements_by_name(Grid &grid){
	std::cerr << "Sorting Elements By Name" << std::endl;
	sort(grid.elements.begin(),grid.elements.end(),compare_element_by_name);
};

void sort_elements_by_index(Grid &grid){
	std::cerr << "Sorting Elements By Index" << std::endl;
	sort(grid.elements.begin(),grid.elements.end(),compare_element_by_index);
};
