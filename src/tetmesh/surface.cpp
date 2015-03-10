#include "surface.h"

#include "unstruc.h"
#include "volume.h"

#include <tetgen.h>

#include <algorithm>
#include <list>
#include <algorithm>
#include <vector>
#include <utility>

using namespace unstruc;

namespace tetmesh {

Grid surfacegrid_from_tetgenio(tetgenio const& tg) {
	Grid grid (3);
	grid.points.reserve(tg.numberofpoints);
	for (int i = grid.points.size(); i < tg.numberofpoints; ++i) {
		Point p;
		p.x = tg.pointlist[3*i];
		p.y = tg.pointlist[3*i+1];
		p.z = tg.pointlist[3*i+2];
		grid.points.push_back(p);
	}
	grid.elements.reserve(tg.numberoftrifaces);
	for (int i = 0; i < tg.numberoftrifaces; ++i) {
		Element e (Shape::Triangle);
		e.name_i = tg.trifacemarkerlist[i];
		e.points[0] = tg.trifacelist[3*i];
		e.points[1] = tg.trifacelist[3*i+1];
		e.points[2] = tg.trifacelist[3*i+2];
		grid.elements.push_back(e);
	}
	grid.names[0].dim = 2;
	return grid;
}

Grid tetrahedralize_surface(Grid const& surface, double max_area) {
	tetgenio in;
	in.mesh_dim = 3;
	in.firstnumber = 0;

	in.numberofpoints = surface.points.size();
	in.pointlist  = new REAL[surface.points.size()*3];
	int i = 0;
	for (Point const& p : surface.points) {
		in.pointlist[i] = p.x;
		in.pointlist[i+1] = p.y;
		in.pointlist[i+2] = p.z;
		i += 3;
	}

	in.numberoffacets = surface.elements.size();
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];

	for (int i = 0; i < surface.elements.size(); ++i) {
		Element const& e = surface.elements[i];
		tetgenio::facet& f = in.facetlist[i];
		tetgenio::init(&f);

		in.facetmarkerlist[i] = e.name_i;
		
		f.numberofpolygons = 1;
		f.polygonlist = new tetgenio::polygon[1];
		tetgenio::polygon& p = f.polygonlist[0];
		tetgenio::init(&p);
		assert (Shape::Info[e.type].dim == 2);

		p.numberofvertices = e.points.size();
		p.vertexlist = new int[e.points.size()];
		for (int j = 0; j < e.points.size(); ++j)
			p.vertexlist[j] = e.points[j];
	}

	in.facetconstraintlist = new REAL[2];
	in.numberoffacetconstraints = 1;
	in.facetconstraintlist[0] = 0;
	in.facetconstraintlist[1] = max_area;

	tetgenbehavior tg;
	tg.plc = 1;
	//tg.nobisect = 1;
	tg.quiet = 1;
	tg.quality = 1;
	//tg.minratio = 1.1;
	//tg.mindihedral = 0;
	//tg.verbose = 1;

	tetgenio out;
	tetrahedralize(&tg,&in,&out,NULL,NULL);
	return surfacegrid_from_tetgenio(out);
}

struct Edge {
	std::pair <int,int> points;
	std::pair <int,int> elements;
	Edge() {};
	Edge(int p1, int p2, int element1, int element2) : points(p1,p2), elements(element1,element2) {};
	bool operator<(const Edge& other) const { return points < other.points; };
};

void verify_complete_surfaces(const Grid& surface) {
	std::vector < Edge > edges;
	std::vector < std::vector<Edge> > edges_per_point (surface.points.size());
	for (int i = 0; i < surface.elements.size(); ++i) {
		const Element& e = surface.elements[i];
		if (Shape::Info[e.type].dim != 2)
			fatal("Not a surface. Has non-surface elements");
		for (int j = 0; j < e.points.size(); ++j) {
			int j2 = (j + 1)%e.points.size();
			int p = e.points[j];
			int p2 = e.points[j2];
			if (p < p2) {
				edges.push_back( Edge(p,p2,-1,-1) );
				edges_per_point[p].push_back( Edge(p,p2,i,-1) );
				edges_per_point[p2].push_back( Edge(p,p2,i,-1) );
			} else {
				edges.push_back( Edge(p2,p,-1,-1) );
				edges_per_point[p].push_back( Edge(p2,p,i,-1) );
				edges_per_point[p2].push_back( Edge(p2,p,i,-1) );
			}
		}
	}
	std::sort(edges.begin(),edges.end());
	for (int i = 0; i < edges.size(); ++i) {
		if (i == edges.size()-1)
			fatal("Boundary Edge found (1)");
		int i2 = i + 1;
		if (edges[i].points != edges[i2].points)
			fatal("Boundary Edge found (2)");
		if (i + 2 < edges.size()) {
			int i3 = i + 2;
			if (edges[i2].points == edges[i3].points)
				fatal("Non-Manifold Edge Found");
		}
		++i;
	}
	for (std::vector <Edge> edges : edges_per_point) {
		if (edges.size()%2 != 0)
			fatal("Boundary Edge found (3)");
		std::sort(edges.begin(),edges.end());
		std::list <Edge> edge_list;
		for (int i = 0; i < edges.size()/2; ++i) {
			const Edge& ee1 = edges[2*i];
			const Edge& ee2 = edges[2*i+1];
			if (ee1.points != ee2.points)
				fatal("Boundary Edge found (4)");
			edge_list.push_back( Edge(ee1.points.first,ee2.points.second,ee1.elements.first,ee2.elements.first) );
		}
		std::list <int> surface_list;
		int initial_size = edge_list.size();
		for (int i = 0; i < initial_size; ++i) {
			if (edge_list.size() == 0) break;
			auto it = edge_list.begin();
			while (it != edge_list.end()) {
				Edge& ee = *it;
				if (surface_list.size() == 0) {
					surface_list.push_back(ee.elements.first);
					surface_list.push_back(ee.elements.second);
					it = edge_list.erase(it);
				} else if (ee.elements.first == surface_list.front()) {
					surface_list.push_front(ee.elements.second);
					it = edge_list.erase(it);
				} else if (ee.elements.second == surface_list.front()) {
					surface_list.push_front(ee.elements.first);
					it = edge_list.erase(it);
				} else if (ee.elements.first == surface_list.back()) {
					surface_list.push_back(ee.elements.second);
					it = edge_list.erase(it);
				} else if (ee.elements.second == surface_list.back()) {
					surface_list.push_back(ee.elements.first);
					it = edge_list.erase(it);
				} else {
					it++;
				}
			}
		}
		if (surface_list.front() != surface_list.back())
			fatal("Boundary Edge Found (5)");
		if (edge_list.size() > 0)
			fatal("Non-manifold Point Found");
	}
}

std::vector <Point> orient_surfaces(Grid& surface) {
	verify_complete_surfaces(surface);

	Intersections intersections = Intersections::find(surface);
	if (intersections.points.size() || intersections.elements.size()) {
		fatal("Intersections found in surface");
	}

	Grid vol = volgrid_from_surface(surface);

	std::vector< std::vector<bool> > surface_map;

	bool found_point = false;
	for (const Element& e : surface.elements) {
		if (e.type != Shape::Triangle) fatal("orient_surface only works with triangles currently");
		bool match = false;
		for (std::vector<bool>& s : surface_map) {
			if (s[e.points[0]] || s[e.points[1]] || s[e.points[2]]) {
				s[e.points[0]] = true;
				s[e.points[1]] = true;
				s[e.points[2]] = true;
				match = true;
				break;
			}
		}
		if (!match) {
			std::vector<bool> s ( surface.points.size(), false );
			s[e.points[0]] = true;
			s[e.points[1]] = true;
			s[e.points[2]] = true;
			surface_map.push_back(s);
		}
	}
	int n_surfaces = 0;
	for (int i = 0; i < surface_map.size(); ++i) {
		std::vector<bool>& s1 = surface_map[i];
		bool deleted = false;
		for (int j = i+1; j < surface_map.size(); ++j) {
			std::vector<bool>& s2 = surface_map[j];
			bool match = false;
			for (int k = 0; k < s1.size(); ++k) {
				if (s1[k] && s2[k]) {
					match = true;
					break;
				}
			}
			if (match) {
				for (int k = 0; k < s1.size(); ++k) {
					if (s1[k]) s2[k] = true;
				}
				deleted = true;
				break;
			}
		}
		if (!deleted) {
			surface_map[n_surfaces] = surface_map[i];
			n_surfaces++;
		}
	}
	surface_map.resize(n_surfaces);
	if (n_surfaces > 1)
		fprintf(stderr,"%d Surfaces Found\n",n_surfaces);

	std::vector <Point> holes;
	for (const std::vector<bool> s : surface_map) {
		bool hole_found = false;
		for (const Element& e : surface.elements) {
			if (!s[e.points[0]]) continue;
			const Point& p0 = surface.points[e.points[0]];
			const Point& p1 = surface.points[e.points[1]];
			const Point& p2 = surface.points[e.points[2]];
			Point p;
			p.x = (p0.x + p1.x + p2.x)/3;
			p.y = (p0.y + p1.y + p2.y)/3;
			p.z = (p0.z + p1.z + p2.z)/3;
			Vector v1 = p1 - p0;
			Vector v2 = p2 - p1;
			Vector n = cross(v1,v2);
			Point test = p - n*1e-6;
			Point test2 = p + n*1e-6;

			if (vol.test_point_inside(test)) {
				holes.push_back(test);
				hole_found = true;
				break;
			} else if (vol.test_point_inside(test2)) {
#ifndef NDEBUG
				fprintf(stderr,"(tetmesh::orient_surface) Reorienting Surface\n");
#endif
				for (Element& e : surface.elements) {
					if (!s[e.points[0]]) continue;
					if (e.type != Shape::Triangle)
						fatal("(tetmesh::orient_surface) current only works with triangle surfaces");
					std::swap(e.points[1],e.points[2]);
				}
				holes.push_back(test2);
				hole_found = true;
				break;
			}
		}
		if (!hole_found) {
			fatal("Couldn't find hole for one of the surfaces");
		}
	}
	return holes;
}

}
