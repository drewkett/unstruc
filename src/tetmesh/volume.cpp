#include "volume.h"

#include "unstruc.h"

#include <tetgen.h>

Grid grid_from_tetgenio(tetgenio const& tg) {
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
		Element e (Shape::Tetra);
		assert (tg.numberofcorners == 4);
		e.points[0] = tg.tetrahedronlist[4*i];
		e.points[1] = tg.tetrahedronlist[4*i+1];
		e.points[2] = tg.tetrahedronlist[4*i+2];
		e.points[3] = tg.tetrahedronlist[4*i+3];
		grid.elements.push_back(e);
	}
	return grid;
}

Grid volgrid_from_surface(Grid const& surface, Point& hole, double min_ratio) {
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
		if (e.type != Shape::Triangle)
			NotImplemented("Currently only supports triangles");

		p.numberofvertices = e.points.size();
		p.vertexlist = new int[e.points.size()];
		p.vertexlist[0] = e.points[0];
		p.vertexlist[1] = e.points[1];
		p.vertexlist[2] = e.points[2];
	}

	if (&hole != &NullPoint) {
		in.numberofholes = 1;
		in.holelist = new REAL[3];
		in.holelist[0] = hole.x;
		in.holelist[1] = hole.y;
		in.holelist[2] = hole.z;
	}
	tetgenbehavior tg;
	tg.plc = 1;
	tg.nobisect = 1;
	tg.quiet = 1;
	tg.quality = 1;
	if (min_ratio > 1)
		tg.minratio = min_ratio;
	//tg.mindihedral = 0;
	//tg.verbose = 1;

	tetgenio out;
	tetrahedralize(&tg,&in,&out,NULL,NULL);

	return grid_from_tetgenio(out);
}

Point find_point_inside_surface(const Grid& surface) {
	Grid vol = volgrid_from_surface(surface);

	const Element& e = surface.elements[0];
	assert (e.type == Shape::Triangle);
	const Point& p0 = surface.points[e.points[0]];
	const Point& p1 = surface.points[e.points[1]];
	const Point& p2 = surface.points[e.points[2]];
	Vector v1 = p1 - p0;
	Vector v2 = p2 - p1;
	Vector n = cross(v1,v2);
	Point test = p0 + n/1000;
	Point test2 = p0 - n/1000;

	if (vol.test_point_inside(test))
		return test;
	else if (vol.test_point_inside(test2))
		return test2;
	else 
		Fatal("Not sure what to do");

	return Point();
}
