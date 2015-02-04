#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <cmath>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>

void merge_points(Grid &grid, double tol) {
	std::cerr << "Merging Points" << std::endl;
	int n_merged = 0;
	int n_points = grid.points.size();

	std::cerr << "Sorting Points By Location" << std::endl;
	std::vector< std::pair<double,int> > s (n_points);
	for (int i = 0; i < n_points; ++i) {
		Point& p = grid.points[i];
		s[i] = std::make_pair(p.x+p.y+p.z,i);
	}

	sort(s.begin(),s.end());

	std::vector<int> merged_index (n_points);
	for (int i = 0; i < n_points; i++)
		merged_index[i] = i;

	std::cerr << "Comparing Points" << std::endl;
	for (int _i = 0; _i < n_points; _i++) {
		double si = s[_i].first;
		int i = s[_i].second;
		Point& p1 = grid.points[i];
		for (int _j = _i+1; _j < n_points; _j++) {
			double sj = s[_j].first;
			int j = s[_j].second;
			// Check if point has already been merged
			if (merged_index[j] != j) continue;

			Point& p2 = grid.points[j];
			if (fabs(si - sj) > 3*tol) break;
			if (same(p1,p2,tol)) {
				merged_index[j] = i;
				n_merged++;
			}
		}
	}

	std::cerr << "Assembling Final Index" << std::endl;
	std::vector<int> new_index (n_points);
	for (int i = 0; i < n_points; ++i)
		new_index[i] = -1;
	int new_i = 0;
	for (int i = 0; i < n_points; ++i) {
		if (merged_index[i] == i) {
			grid.points[new_i] = grid.points[i];
			new_index[i] = new_i;
			new_i++;
		}
	}
	grid.points.resize(new_i);
	for (int i = 0; i < n_points; ++i) {
		if (merged_index[i] != i)
			new_index[i] = new_index[merged_index[i]];
	}
	std::cerr << n_merged << " Points Merged" << std::endl;
	std::cerr << "Updating Elements" << std::endl;
	for (Element& e : grid.elements)
		for (int& p : e.points)
			p = new_index[p];
}

void delete_inner_faces(Grid &grid) {
	std::cerr << "Deleting Inner Faces" << std::endl;
	int n_elements = grid.elements.size();
	int n_points = grid.points.size();

	std::cerr << "Sorting Points By Location" << std::endl;
	std::vector< std::pair<int,int> > s (n_elements);
	for (int i = 0; i < n_elements; ++i) {
		int min_p = n_points;
		for (int p : grid.elements[i].points)
			if (p < min_p)
				min_p = p;
		s[i] = std::make_pair(min_p,i);
	}

	sort(s.begin(),s.end());

	std::vector<bool> deleted_index (n_elements);
	for (int i = 0; i < n_elements; i++)
		deleted_index[i] = false;

	fprintf(stderr,"Comparing Points\n");
	for (int _i = 0; _i < n_elements; _i++) {
		int si = s[_i].first;
		int i = s[_i].second;
		Element &ei = grid.elements[i];
		if (ei.dim == grid.dim) continue;
		for (int _j = _i+1; _j < n_elements; _j++) {
			int sj = s[_j].first;
			int j = s[_j].second;
			Element &ej = grid.elements[j];
			if (ej.dim == grid.dim) continue;
			if (deleted_index[j]) continue;
		    if (si != sj) break;
			if (same(ei,ej)) {
				deleted_index[i] = true;
				deleted_index[j] = true;
			}
		}
	}
	int n_deleted = 0;
	int new_i = 0;
	for (int i = 0; i < n_elements; ++i) {
		if (deleted_index[i]) {
			n_deleted++;
		} else {
			grid.elements[new_i] = grid.elements[i];
			new_i++;
		}
	}
	grid.elements.resize(new_i);
	fprintf(stderr,"%d Faces Deleted\n",n_deleted);
}

void collapse_elements(Grid &grid) {
	int n_elements = grid.elements.size();

	std::vector<Element> new_elements;
	int n_collapsed = 0;
	int n_deleted = 0;
	std::vector<bool> deleted_elements (n_elements,false);
	for (int i = 0; i < n_elements; ++i) {
		Element& e = grid.elements[i];
		if (can_collapse(e)) {
			bool deleted = collapse(e,new_elements);
			if (deleted) {
				n_deleted++;
				deleted_elements[i] = true;
			}
			n_collapsed++;
		}
	}
	int new_i = 0;
	for (int i = 0; i < n_elements; ++i) {
		if (!deleted_elements[i]) {
			grid.elements[new_i] = grid.elements[i];
			new_i++;
		}
	}
	grid.elements.resize(new_i);

	int n_added = new_elements.size();
	grid.elements.insert(grid.elements.end(),new_elements.begin(),new_elements.end());

	std::cerr << n_collapsed << " Elements Collapsed" << std::endl;
	std::cerr << n_added << " Elements Created On Collapse" << std::endl;
	std::cerr << n_deleted << " Elements Deleted On Collapse" << std::endl;
};
