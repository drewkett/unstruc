
#include <cstdio>
#include <algorithm>

#include <tetgen.h>

#include "error.h"
#include "element.h"
#include "io.h"
#include "grid.h"

void print_usage() {
	fprintf(stderr, "volmesh farfield_stl offset_su2 output_name\n");
}

Grid grid_from_tetgenio(tetgenio& tg) {
	Grid grid (3);
	grid.points.reserve(tg.numberofpoints);
	for (int i = grid.points.size(); i < tg.numberofpoints; ++i) {
		Point p;
		p.x = tg.pointlist[3*i];
		p.y = tg.pointlist[3*i+1];
		p.z = tg.pointlist[3*i+2];
		grid.points.push_back(p);
	}
	grid.elements.reserve(grid.elements.size()+tg.numberoftetrahedra);
	for (int i = 0; i < tg.numberoftetrahedra; ++i) {
		Element e (TETRA);
		assert (tg.numberofcorners == 4);
		e.points[0] = tg.tetrahedronlist[4*i];
		e.points[1] = tg.tetrahedronlist[4*i+1];
		e.points[2] = tg.tetrahedronlist[4*i+2];
		e.points[3] = tg.tetrahedronlist[4*i+3];
		grid.elements.push_back(e);
	}
	return grid;
}

Grid tetrahedralize_surface(Grid& surface, double max_area) {
	tetgenio in;
	in.mesh_dim = 3;
	in.firstnumber = 0;

	in.numberofpoints = surface.points.size();
	in.pointlist  = new REAL[surface.points.size()*3];
	int i = 0;
	for (Point& p : surface.points) {
		in.pointlist[i] = p.x;
		in.pointlist[i+1] = p.y;
		in.pointlist[i+2] = p.z;
		i += 3;
	}

	in.numberoffacets = surface.elements.size();
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];

	for (int i = 0; i < surface.elements.size(); ++i) {
		Element& e = surface.elements[i];
		tetgenio::facet& f = in.facetlist[i];
		tetgenio::init(&f);

		in.facetmarkerlist[i] = e.name_i;
		
		f.numberofpolygons = 1;
		f.polygonlist = new tetgenio::polygon[1];
		tetgenio::polygon& p = f.polygonlist[0];
		tetgenio::init(&p);
		assert (e.dim == 2);

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
	tg.verbose = 1;

	tetgenio out;
	tetrahedralize(&tg,&in,&out,NULL,NULL);

	return grid_from_tetgenio(out);
}

Grid volgrid_from_surface(Grid& surface) {
	tetgenio in;
	in.mesh_dim = 3;
	in.firstnumber = 0;

	in.numberofpoints = surface.points.size();
	in.pointlist  = new REAL[surface.points.size()*3];
	int i = 0;
	for (Point& p : surface.points) {
		in.pointlist[i] = p.x;
		in.pointlist[i+1] = p.y;
		in.pointlist[i+2] = p.z;
		i += 3;
	}

	in.numberoffacets = surface.elements.size();
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];

	for (int i = 0; i < surface.elements.size(); ++i) {
		Element& e = surface.elements[i];
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

	tetgenbehavior tg;
	tg.plc = 1;
	tg.nobisect = 1;
	//tg.quiet = 1;
	//tg.quality = 1;
	//tg.minratio = 1.1;
	//tg.mindihedral = 0;
	//tg.verbose = 1;

	tetgenio out;
	tetrahedralize(&tg,&in,&out,NULL,NULL);

	return grid_from_tetgenio(out);
}

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

Grid create_farfield_box(Grid& surface) {
	double min_x = 1e10;
	double min_y = 1e10;
	double min_z = 1e10;
	double max_x = -1e10;
	double max_y = -1e10;
	double max_z = -1e10;
	for (Point& p : surface.points) {
		if (p.x < min_x) min_x = p.x;
		if (p.y < min_y) min_y = p.y;
		if (p.z < min_z) min_z = p.z;
		if (p.x > max_x) max_x = p.x;
		if (p.y > max_y) max_y = p.y;
		if (p.z > max_z) max_z = p.z;
	}
	double dx = max_x-min_x;
	double dy = max_y-min_y;
	double dz = max_z-min_z;
	double max_length = std::max(std::max(dx,dy),dz);
	double delta = 10*max_length;
	min_x -= delta;
	min_y -= delta;
	min_z -= delta;
	max_x += delta;
	max_y += delta;
	max_z += delta;
	Grid farfield (3);
	farfield.points.emplace_back(min_x, min_y, min_z);
	farfield.points.emplace_back(max_x, min_y, min_z);
	farfield.points.emplace_back(max_x, max_y, min_z);
	farfield.points.emplace_back(min_x, max_y, min_z);
	farfield.points.emplace_back(min_x, min_y, max_z);
	farfield.points.emplace_back(max_x, min_y, max_z);
	farfield.points.emplace_back(max_x, max_y, max_z);
	farfield.points.emplace_back(min_x, max_y, max_z);
	Element e1 (QUAD);
	e1.points = {0,1,2,3};
	farfield.elements.push_back(e1);
	Element e2 (QUAD);
	e2.points = {4,5,6,7};
	farfield.elements.push_back(e2);
	Element e3 (QUAD);
	e3.points = {0,1,5,4};
	farfield.elements.push_back(e3);
	Element e4 (QUAD);
	e4.points = {2,3,7,6};
	farfield.elements.push_back(e4);
	Element e5 (QUAD);
	e5.points = {1,2,6,5};
	farfield.elements.push_back(e5);
	Element e6 (QUAD);
	e6.points = {3,0,4,7};
	farfield.elements.push_back(e6);

	double max_area = 4*(max_length*max_length);
	Grid tetra_farfield = tetrahedralize_surface(farfield,max_area);
	return farfield;
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
	write_grid(outputname+".vtk",grid);
	write_grid(outputname+".su2",grid);
}
