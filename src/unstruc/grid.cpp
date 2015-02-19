#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <cassert>
#include <cmath>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>

Grid::Grid(int _dim) : dim(_dim) {
	names.emplace_back(dim, "default");
}

void Grid::merge_points(double tol) {
	std::cerr << "Merging Points" << std::endl;
	int n_merged = 0;
	int n_points = this->points.size();

	std::cerr << "Sorting Points By Location" << std::endl;
	std::vector< std::pair<double,int> > s (n_points);
	for (int i = 0; i < n_points; ++i) {
		Point& p = this->points[i];
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
		Point& p1 = this->points[i];
		for (int _j = _i+1; _j < n_points; _j++) {
			double sj = s[_j].first;
			int j = s[_j].second;
			// Check if point has already been merged
			if (merged_index[j] != j) continue;

			Point& p2 = this->points[j];
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
			this->points[new_i] = this->points[i];
			new_index[i] = new_i;
			new_i++;
		}
	}
	this->points.resize(new_i);
	for (int i = 0; i < n_points; ++i) {
		if (merged_index[i] != i)
			new_index[i] = new_index[merged_index[i]];
	}
	std::cerr << n_merged << " Points Merged" << std::endl;
	std::cerr << "Updating Elements" << std::endl;
	for (Element& e : this->elements)
		for (int& p : e.points)
			p = new_index[p];
}

void Grid::delete_inner_faces() {
	std::cerr << "Deleting Inner Faces" << std::endl;
	int n_elements = this->elements.size();
	int n_points = this->points.size();

	std::cerr << "Sorting Points By Location" << std::endl;
	std::vector< std::pair<int,int> > s (n_elements);
	for (int i = 0; i < n_elements; ++i) {
		int min_p = n_points;
		for (int p : this->elements[i].points)
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
		Element &ei = this->elements[i];
		if (ei.dim == this->dim) continue;
		for (int _j = _i+1; _j < n_elements; _j++) {
			int sj = s[_j].first;
			int j = s[_j].second;
			Element &ej = this->elements[j];
			if (ej.dim == this->dim) continue;
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
			this->elements[new_i] = this->elements[i];
			new_i++;
		}
	}
	this->elements.resize(new_i);
	fprintf(stderr,"%d Faces Deleted\n",n_deleted);
}

void Grid::collapse_elements() {
	int n_elements = this->elements.size();

	std::vector<Element> new_elements;
	int n_collapsed = 0;
	int n_deleted = 0;
	std::vector<bool> deleted_elements (n_elements,false);
	for (int i = 0; i < n_elements; ++i) {
		Element& e = this->elements[i];
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
			this->elements[new_i] = this->elements[i];
			new_i++;
		}
	}
	this->elements.resize(new_i);

	int n_added = new_elements.size();
	this->elements.insert(this->elements.end(),new_elements.begin(),new_elements.end());

	std::cerr << n_collapsed << " Elements Collapsed" << std::endl;
	std::cerr << n_added << " Elements Created On Collapse" << std::endl;
	std::cerr << n_deleted << " Elements Deleted On Collapse" << std::endl;
};

Grid Grid::grid_from_elements(std::vector<Element>& elements) {
	Grid g (3);
	std::vector<int> index (points.size(),-1);
	int n_points = 0;
	for (Element &e_orig : elements) {
		Element e (e_orig.type);
		e.points.resize(e_orig.points.size());
		for (int i = 0; i < e_orig.points.size(); ++i) {
			int p = e_orig.points[i];
			if (index[p] == -1) {
				index[p] = n_points;
				g.points.push_back(points[p]);
				n_points++;
			}
			e.points[i] = index[p];
		}
		g.elements.push_back(e);
	}
	return g;
}

void Grid::add_grid(Grid& other) {
	assert (dim == other.dim);
	int point_offset = points.size();
	int name_offset = names.size();
	points.insert(points.end(),other.points.begin(),other.points.end());
	names.insert(names.end(),other.names.begin(),other.names.end());
	elements.reserve(elements.size() + other.elements.size());
	for (Element e : other.elements) {
		for (int i = 0; i < e.points.size(); ++i) {
			e.points[i] += point_offset;
		}
		e.name_i += name_offset;
		elements.push_back(e);
	}
}

void Grid::delete_empty_names() {
	bool name_exists [names.size()];
	for (int i = 0; i < names.size(); ++i)
		name_exists[i] = false;

	for (Element& e: elements) {
		name_exists[e.name_i] = true;
	}

	int name_map [names.size()];
	int i = 0;
	for (int _i = 0; _i < names.size(); ++_i)  {
		if (name_exists[_i]) {
			name_map[_i] = i;
			names[i] = names[_i];
			i++;
		} else
			name_map[_i] = -1;
	}
	names.resize(i);

	for (Element& e: elements)
		e.name_i = name_map[e.name_i];
}