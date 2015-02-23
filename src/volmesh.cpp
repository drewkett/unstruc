
#include <cstdio>
#include <algorithm>

#include <tetgen.h>
#include "unstruc.h"
#include "tetmesh.h"

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

	Point hole = find_point_inside_surface(offset);

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
		if (e.type != Shape::Triangle)
			Fatal("(unstruc-volmesh) Only Triangle meshes supported as surface");

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
		Element e (Shape::Tetra);
		e.points[0] = out.tetrahedronlist[4*i];
		e.points[1] = out.tetrahedronlist[4*i+1];
		e.points[2] = out.tetrahedronlist[4*i+2];
		e.points[3] = out.tetrahedronlist[4*i+3];
		grid.elements.push_back(e);
	}

	grid.delete_empty_names();
	write_grid(outputname,grid);
}
