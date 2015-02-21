
#include <cstdio>
#include <algorithm>

#include <tetgen.h>
#include "unstruc.h"
#include "tetmesh.h"

bool test_point_in_volume(Grid& vol, Point& p) {
	bool inside = false;
	for (Element& e : vol.elements) {
		Point& p0 = vol.points[e.points[0]];
		Point& p1 = vol.points[e.points[1]];
		Point& p2 = vol.points[e.points[2]];
		Point& p3 = vol.points[e.points[3]];

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

Point find_point_in_surface(Grid& surface) {
	Grid vol = volgrid_from_surface(surface);

	Element& e = surface.elements[0];
	assert (e.type == TRI);
	Point& p0 = surface.points[e.points[0]];
	Point& p1 = surface.points[e.points[1]];
	Point& p2 = surface.points[e.points[2]];
	Vector v1 = p1 - p0;
	Vector v2 = p2 - p1;
	Vector n = cross(v1,v2);
	Point test = p0 + n/1000;
	Point test2 = p0 - n/1000;

	if (test_point_in_volume(vol,test))
		return test;
	else if (test_point_in_volume(vol,test2))
		return test2;
	else 
		Fatal("Not sure what to do");

	return Point();
}

void print_usage () {
	fprintf(stderr,
"unstruc-volmesh surface_file output_file\n\n"
"This tool creates a volume mesh, including creating a farfield surface, using an input surface mesh\n");
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		print_usage();
		fprintf(stderr,"\nMust pass 2 arguments\n");
		return 1;
	}

	std::string offsetname (argv[1]);
	std::string outputname (argv[2]);

	Grid offset = read_grid(offsetname);
	offset.merge_points(0);
	write_grid("offset.vtk",offset);

	Grid farfield = create_farfield_box(offset);
	write_grid("farfield.vtk",farfield);

	Point hole = find_point_in_surface(offset);

	Grid& grid = offset;
	grid.add_grid(farfield);
	write_grid("input.vtk",grid);

	//int i_edge = -1;
	//for (int i = 0; i < grid.names.size(); ++i) {
	//	if (grid.names[i].name == "Edge") {
	//		i_edge = i;
	//		break;
	//	}
	//}
	//assert(i_edge != -1);
	//std::vector <Element> edge_elements;
	//for (Element& e : grid.elements) {
	//	if (e.name_i == 1 || e.name_i == i_edge)
	//		edge_elements.push_back(e);
	//}

	std::vector <Element> edge_elements = grid.elements;

	tetgenio in;
	in.mesh_dim = grid.dim;

	in.numberofpoints = grid.points.size();
	in.pointlist  = new REAL[grid.points.size()*3];
	int i = 0;
	for (Point& p : grid.points) {
		in.pointlist[i] = p.x;
		in.pointlist[i+1] = p.y;
		in.pointlist[i+2] = p.z;
		i += 3;
	}

	in.firstnumber = 0;
	in.numberoffacets = edge_elements.size();
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];

	for (int i = 0; i < edge_elements.size(); ++i) {
		Element& e = edge_elements[i];
		tetgenio::facet& f = in.facetlist[i];
		tetgenio::init(&f);

		in.facetmarkerlist[i] = e.name_i;
		
		f.numberofpolygons = 1;
		f.polygonlist = new tetgenio::polygon[1];
		tetgenio::polygon& p = f.polygonlist[0];
		tetgenio::init(&p);
		assert (e.type == TRI);
		assert (e.points.size() == 3);

		p.numberofvertices = e.points.size();
		p.vertexlist = new int[e.points.size()];
		p.vertexlist[0] = e.points[0];
		p.vertexlist[1] = e.points[1];
		p.vertexlist[2] = e.points[2];
	}

	in.numberofholes = 1;
	in.holelist = new REAL[3];
	in.holelist[0] = hole.x;
	in.holelist[1] = hole.y;
	in.holelist[2] = hole.z;

	tetgenbehavior tg;
	tg.plc = 1;
	tg.nobisect = 1;
	tg.quality = 1;
	tg.minratio = 1.03;
	tg.mindihedral = 0;
	//tg.verbose = 1;
	tg.quiet = 1;
	tetgenio out;
	tetrahedralize(&tg,&in,&out,NULL,NULL);

	grid.points.reserve(out.numberofpoints);
	for (int i = grid.points.size(); i < out.numberofpoints; ++i) {
		Point p;
		p.x = out.pointlist[3*i];
		p.y = out.pointlist[3*i+1];
		p.z = out.pointlist[3*i+2];
		grid.points.push_back(p);
	}
	grid.elements.reserve(grid.elements.size()+out.numberoftetrahedra);
	for (int i = 0; i < out.numberoftetrahedra; ++i) {
		Element e (TETRA);
		assert (out.numberofcorners == 4);
		e.points[0] = out.tetrahedronlist[4*i];
		e.points[1] = out.tetrahedronlist[4*i+1];
		e.points[2] = out.tetrahedronlist[4*i+2];
		e.points[3] = out.tetrahedronlist[4*i+3];
		grid.elements.push_back(e);
	}

	grid.delete_empty_names();
	write_grid(outputname,grid);
}
