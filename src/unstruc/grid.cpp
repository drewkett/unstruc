#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <cassert>
#include <cmath>
#include <cfloat>
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
	int n_points = points.size();

	std::cerr << "Sorting Points By Location" << std::endl;
	std::vector< std::pair<double,int> > s (n_points);
	for (int i = 0; i < n_points; ++i) {
		Point& p = points[i];
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
		Point& p1 = points[i];
		for (int _j = _i+1; _j < n_points; _j++) {
			double sj = s[_j].first;
			int j = s[_j].second;
			// Check if point has already been merged
			if (merged_index[j] != j) continue;

			Point& p2 = points[j];
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
			points[new_i] = points[i];
			new_index[i] = new_i;
			new_i++;
		}
	}
	points.resize(new_i);
	for (int i = 0; i < n_points; ++i) {
		if (merged_index[i] != i)
			new_index[i] = new_index[merged_index[i]];
	}
	std::cerr << n_merged << " Points Merged" << std::endl;
	std::cerr << "Updating Elements" << std::endl;
	for (Element& e : elements)
		for (int& p : e.points)
			p = new_index[p];
}

void Grid::delete_inner_faces() {
	std::cerr << "Deleting Inner Faces" << std::endl;
	int n_elements = elements.size();
	int n_points = points.size();

	std::cerr << "Sorting Points By Location" << std::endl;
	std::vector< std::pair<int,int> > s (n_elements);
	for (int i = 0; i < n_elements; ++i) {
		int min_p = n_points;
		for (int p : elements[i].points)
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
		Element &ei = elements[i];
		if (Shape::Info[ei.type].dim == dim) continue;
		for (int _j = _i+1; _j < n_elements; _j++) {
			int sj = s[_j].first;
			int j = s[_j].second;
			Element &ej = elements[j];
			if (Shape::Info[ej.type].dim == dim) continue;
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
			elements[new_i] = Element (elements[i]);
			new_i++;
		}
	}
	elements.resize(new_i);
	fprintf(stderr,"%d Faces Deleted\n",n_deleted);
}

void Grid::collapse_elements() {
	int n_elements = elements.size();

	std::vector<Element> new_elements;
	int n_collapsed = 0;
	int n_deleted = 0;
	std::vector<bool> deleted_elements (n_elements,false);
	for (int i = 0; i < n_elements; ++i) {
		Element& e = elements[i];
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
			elements[new_i] = elements[i];
			new_i++;
		}
	}
	elements.resize(new_i);

	int n_added = new_elements.size();
	elements.insert(elements.end(),new_elements.begin(),new_elements.end());

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

Grid& Grid::operator+=(const Grid& other) {
	if (dim != other.dim)
		Fatal("Dimensions must match");
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
	return *this;
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

bool Grid::test_point_inside(Point const& p) {
	bool inside = false;
	for (Element const& e : elements) {
		if (e.type != Shape::Tetra)
			NotImplemented("test_point_inside_volume only surpports Tetra's");
		Point const& p0 = points[e.points[0]];
		Point const& p1 = points[e.points[1]];
		Point const& p2 = points[e.points[2]];
		Point const& p3 = points[e.points[3]];

		Vector v01 = p1 - p0;
		Vector v12 = p2 - p1;
		Vector v20 = p0 - p2;
		Vector v03 = p3 - p0;
		Vector v13 = p3 - p1;
		Vector v23 = p3 - p2;
		
		double test1 = dot(p-p0,cross(v01,v12));
		assert(test1 != 0);
		double test2 = -dot(p-p0,cross(v01,v13));
		assert(test2 != 0);
		if ((test1 > 0) != (test2 > 0))
			continue;
		double test3 = -dot(p-p1,cross(v12,v23));
		assert(test3 != 0);
		if ((test2 > 0) != (test3 > 0))
			continue;
		double test4 = -dot(p-p2,cross(v20,v03));
		assert(test4 != 0);
		if ((test3 > 0) != (test4 > 0))
			continue;
		inside = true;
		break;
	}
	return inside;
}

bool Grid::check_integrity() const {
	if (dim < 2 || dim > 3) {
		fprintf(stderr,"Grid dim incorrectly set\n");
		return false;
	}
	int n_points = points.size();
	for (const Element& e : elements) {
		if (e.type == Shape::Undefined) {
			fprintf(stderr,"Undefined shape\n");
			return false;
		}
		Shape s = Shape::Info[e.type];
		if (s.n_points && s.n_points != e.points.size()) {
			fprintf(stderr,"Element has wrong number of points\n");
			return false;
		}
		if (e.name_i >= names.size()) {
			fprintf(stderr,"Element has invalid name (%d >= %lu)\n",e.name_i,names.size());
			return false;
		}
		for (int p : e.points)
			if (p >= n_points) {
				fprintf(stderr,"Element uses non existent point\n");
				return false;
			}
	}
	for (const Name& name : names) {
		if (name.dim < 2 || name.dim > 3) {
			fprintf(stderr,"Name dimension not correctly set\n");
			return false;
		}
	}
	return true;
}

Point Grid::get_bounding_min() const {
	Point min ( DBL_MAX, DBL_MAX, DBL_MAX );
	for (Point const& p : points) {
		if (p.x < min.x) min.x = p.x;
		if (p.y < min.y) min.y = p.y;
		if (p.z < min.z) min.z = p.z;
	}
	return min;
}

Point Grid::get_bounding_max() const {
	Point max ( DBL_MIN, DBL_MIN, DBL_MIN );
	for (Point const& p : points) {
		if (p.x > max.x) max.x = p.x;
		if (p.y > max.y) max.y = p.y;
		if (p.z > max.z) max.z = p.z;
	}
	return max;
}
