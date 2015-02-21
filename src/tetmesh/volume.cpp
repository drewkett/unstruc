#include "tetmesh.h"

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
