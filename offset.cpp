
#include "vtk.h"
#include "stl.h"
#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

int main() {
	Grid grid = read_stl_ascii("ac.stl");
	grid.merge_points(0);
	grid.collapse_elements();;
	printf("%zu triangles\n",grid.elements.size());
	printf("%zu points\n",grid.points.size());

	std::vector< Vector > normals;
	normals.resize(grid.elements.size());

	std::vector< Point > centers;
	centers.resize(grid.elements.size());

	std::vector< std::vector <int> > point_elements;
	point_elements.resize(grid.points.size());

	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e = grid.elements[i];

		Point& p0 = grid.points[e.points[0]];
		Point& p1 = grid.points[e.points[1]];
		Point& p2 = grid.points[e.points[2]];

		Vector v1 = p1 - p0;
		Vector v2 = p2 - p1;
		normals[i] = cross(v1,v2)/6;

		centers[i].x = (p0.x + p1.x + p2.x)/3;
		centers[i].y = (p0.y + p1.y + p2.y)/3;
		centers[i].z = (p0.z + p1.z + p2.z)/3;

		for (int p : e.points)
			point_elements[p].push_back(i);
	}

	std::vector <Vector> point_normals (grid.points.size());
	for (int i = 0; i < grid.points.size(); ++i) {
		Point& p = grid.points[i];
		std::vector<int>& elements = point_elements[i];

		Vector total_norm;
		for (int j : elements) {
			Vector& n = normals[j];
			Point& c = centers[j];

			Vector v = p - c;
			Vector _n = n/dot(v,v);
			total_norm += _n;
		}
		total_norm /= total_norm.length();
		point_normals[i] = total_norm;
	}

	Grid offset (3);
	offset.elements = grid.elements;
	for (int i = 0; i < grid.points.size(); ++i) {
		Point& p = grid.points[i];
		Vector& n = point_normals[i];

		Point offset_p = p - n*0.001;
		offset.points.push_back(offset_p);
	}

	toVTK("offset.vtk",offset,true);
	Grid volume (3);
	int n_points = grid.points.size();
	volume.points = grid.points;
	volume.points.insert(volume.points.end(),offset.points.begin(),offset.points.end());

	for (int i = 0; i < grid.elements.size(); ++i) {
		Element& e1 = grid.elements[i];
		Element& e2 = offset.elements[i];
		Element e (WEDGE);
		for (int j = 0; j < 3; ++j) {
			e.points[j] = e1.points[j];
			e.points[j+3] = e2.points[j] + n_points;
		}
		volume.elements.push_back(e);
	}
	toVTK("volume.vtk",volume,true);
	int n_negative_volumes = 0;
	for (Element& e : volume.elements) {
		if (e.calc_volume(volume) < 0)
			n_negative_volumes++;
	}
	printf("%d Negative Volumes\n",n_negative_volumes);
}
