#include "gmsh.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>

namespace unstruc {

void gmsh_write(const std::string& filename, const Grid &grid) {
	FILE* f = fopen(filename.c_str(),"w");
	if (!f) fatal("Could not open file");

	fprintf(f,"$MeshFormat\n");
	fprintf(f,"2.2 0 %lu\n",sizeof(double));
	fprintf(f,"$EndMeshFormat\n");
	fprintf(f,"$Nodes %lu\n",grid.points.size());
	for (int i = 0; i < grid.points.size(); i++) {
		const Point& p = grid.points[i];
		fprintf(f,"%d %.17g %.17g %.17g\n",i+1,p.x,p.y,p.z);
	}
	fprintf(f,"$EndNodes\n");
	fprintf(f,"$Elements %lu\n",grid.elements.size());
	for (int i = 0; i < grid.elements.size(); i++) {
		const Element &e = grid.elements[i];
		int eltype = 0;
		switch (e.type) {
			case Shape::Quad:
				eltype = 3;
				break;
			case Shape::Hexa:
				eltype = 5;
				break;
			default:
				not_implemented("ElType for GMSH");
		}
		fprintf(f,"%d %d 2 %d %d",i+1,eltype,e.name_i+1,e.name_i+1);
		for (int p : e.points)
			fprintf(f," %d",p);
		fprintf(f,"\n");
	}
	fprintf(f,"$EndElements\n");
	fprintf(f,"$PhysicalNames %lu",grid.names.size());
	for (int i = 0; i < grid.names.size(); i++) {
		const Name& name = grid.names[i];
		fprintf(f,"%d %d \"%s\"\n",name.dim,i+1,name.name.c_str());
	}
	fprintf(f,"$EndPhysicalNames\n");
}

} // namespace unstruc::GMSH
