#include "su2.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>

bool toSU2(std::string &outputfile, Grid * grid) {
	int i, j;

	FILE * f;
	f = fopen(outputfile.c_str(),"w");
	if (!f) Fatal("Could not open file");
	Name * name;
	Point * p;
	Element * e;
	set_i(grid);
	std::cerr << "Outputting SU2" << std::endl;
	std::cerr << "Writing Elements" << std::endl;
	fprintf(f,"NDIME= %d\n\n",grid->dim);
	fprintf(f,"NELEM= %d\n",grid->n_elems);
	for (i = 0; i < grid->elements.size(); i++) {
		e = grid->elements[i];
		if (!e or e->dim != grid->dim) continue;
		fprintf(f,"%d",e->type);
		for (j = 0; j < e->len; j++) {
			fprintf(f," %d",(**e->points[j]).i);
		}
		fprintf(f," %d\n",i);
	}
	fprintf(f,"\n");
	std::cerr << "Writing Points" << std::endl;
	fprintf(f,"NPOIN= %d\n",grid->n_points);
	for (i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		p = *grid->ppoints[i];
		if (grid->dim == 2)
			fprintf(f,"%.17g %.17g %d\n",p->x,p->y,p->i);
		else
			fprintf(f,"%.17g %.17g %.17g %d\n",p->x,p->y,p->z,p->i);
	}
	fprintf(f,"\n");
	int n_names = 0;
	for (i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		if (grid->names[i]->dim != 2) continue;
		n_names++;
	}
	std::vector<int> name_count(grid->names.size(),0);
	for (i = 0; i < grid->elements.size(); i++) {
		e = grid->elements[i];
		if (!e or e->dim != (grid->dim-1)) continue;
		if (e->name_i < 0) continue;
		name_count[e->name_i]++;
	}
	std::cerr << "Writing Markers" << std::endl;
	fprintf(f,"NMARK= %d\n",n_names);
	for (i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		name = grid->names[i];
		if (name->dim != grid->dim - 1) continue;
		std::cerr << i << " : " << name->name << std::endl;
		fprintf(f,"MARKER_TAG= %s\n",name->name.c_str());
		fprintf(f,"MARKER_ELEMS= %d\n",name_count[i]);
		for (j = 0; j < grid->elements.size(); j++) {
			e = grid->elements[j];
			if (!e or e->dim != grid->dim - 1) continue;
			if (e->name_i != i) continue;
			fprintf(f,"%d",e->type);
			for (int k = 0; k < e->len; k++) {
				fprintf(f," %d",(**e->points[k]).i);
			}
			fprintf(f,"\n");
		}
	}
	fclose(f);
	return true;
}

Grid * readSU2(std::string &inputfile) {
	int ipoint, ielem, iname, i, j, k;
	int npoint, nelem, nmark;
	Grid * grid = new Grid();
	Element * elem;
	Point * point;
	Name * name;
	std::ifstream f;
	std::istringstream ls;
	std::string line, token;
	f.open(inputfile.c_str(),std::ios::in);
	std::cerr << "Opening SU2 File '" << inputfile << "'" << std::endl;
	if (!f.is_open()) Fatal("Could not open file");
	while (getline(f,line)) {
		std::stringstream ss(line);
		ss >> token;
		if (token.substr(0,6) == "NDIME=") {
			if (token.size() > 6) {
				grid->dim = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				grid->dim = std::atoi(token.c_str());
			}
			std::cerr << grid->dim << " Dimensions" << std::endl;

			//Create default named block to be assigned to all elements
			name = new Name();
			name->name = "default";
			name->dim = grid->dim;
			grid->names.push_back(name);
		} else if (token.substr(0,6) == "NELEM=") {
			if (token.size() > 6) {
				grid->n_elems = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				grid->n_elems = std::atoi(token.c_str());
			}
			std::cerr << grid->n_elems << " Elements" << std::endl;
			grid->elements.resize(grid->n_elems);
			for (int i = 0; i < grid->n_elems; ++i) {
				getline(f,line);
				std::stringstream ss(line);
				ss >> token;
				elem = new Element(atoi(token.c_str()));
				//Assign to default name block
				elem->name_i = 0;
				for (j = 0; j < elem->len; ++j) {
					ss >> token;
					ipoint = std::atoi(token.c_str());
					// Might be able to do this resizing after the fact
					if (ipoint >= grid->points.size()) {
						grid->points.resize(ipoint+1);
						grid->ppoints.resize(ipoint+1);
					}
					elem->points[j] = &grid->points[ipoint];
				}
				if (ss >> token && !ss.eof()) {
					ielem = atoi(token.c_str());
				} else {
					// Need a better plan for unnumbered elements
					if (grid->elements[i])
						Fatal("Not sure what to do with unnumbered element");
					else
						ielem = i;
				}
				if (ielem >= grid->elements.size())
					grid->elements.resize(ielem+1);
				grid->elements[ielem] = elem;
			}
		} else if (token.substr(0,6) == "NPOIN=") {
			if (token.size() > 6) {
				grid->n_points = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				grid->n_points = std::atoi(token.c_str());
			}
			if (ss >> token && !ss.eof())
				grid->n_points = std::atoi(token.c_str());
			std::cerr << grid->n_points << " Points" << std::endl;
			if (grid->n_points > grid->points.size()) {
				grid->points.resize(grid->n_points);
				grid->ppoints.resize(grid->n_points);
			}
			for (i = 0; i < grid->n_points; ++i) {
				getline(f,line);
				std::stringstream ss(line);
				point = new Point();
				ss >> token;
				point->x = std::atof(token.c_str());
				ss >> token;
				point->y = std::atof(token.c_str());
				if (!grid->dim)
					Fatal("Dimension (NDIME) not defined");
				if (grid->dim == 3) {
					ss >> token;
					point->z = std::atof(token.c_str());
				}
				if (ss >> token && !ss.eof()) {
					ss >> token;
					ipoint = std::atoi(token.c_str());
				} else if (!grid->points[ipoint]) {
					ipoint = i;
				} else {
					Fatal("Don't know how to handle unnumbered point");
				}
				point->i = ipoint;
				grid->points[ipoint] = point;
				grid->ppoints[ipoint] = &grid->points[ipoint];
			}
		} else if (token.substr(0,6) == "NMARK=") {
			int nmark;
			if (token.size() > 6) {
				nmark = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				nmark = std::atoi(token.c_str());
			}
			std::cerr << nmark << " Markers" << std::endl;
			for (i = 0; i < nmark; i++) {
				name = new Name();
				name->dim = grid->dim-1;
				iname = grid->names.size();
				grid->names.push_back(name);

				// Read MARKER_TAG=
				getline(f,line);
				std::stringstream ss(line);
				ss >> token;
				if (token.substr(0,11) != "MARKER_TAG=")
					Fatal("Invalid Marker Definition: Expected MARKER_TAG=");
				if (token.size() > 11) {
					name->name.assign(token.substr(11));
				} else {
					ss >> token;
					name->name.assign(token);
				}
				std::cerr << name->name << std::endl;

				// Read MARKER_ELEMS=
				getline(f,line);
				ss.clear();
				ss.str(line);
				ss >> token;
				if (token.substr(0,13) != "MARKER_ELEMS=") 
					Fatal("Invalid Marker Definition: Expected MARKER_ELEMS=");
				if (token.size() > 13) {
					nelem = std::atoi(token.substr(13).c_str());
				} else {
					ss >> token;
					nelem = std::atoi(token.c_str());
				}
				for (j = 0; j < nelem; j++) {
					getline(f,line);
					ss.clear();
					ss.str(line);
					ss >> token;
					elem = new Element(atoi(token.c_str()));
					for (k=0; k<elem->len; k++) {
						ss >> token;
						ipoint = std::atoi(token.c_str());
						if (ipoint >= grid->points.size()) Fatal("Error Marker Element");
						elem->points[k] = &grid->points[ipoint];
					}
					elem->name_i = iname;
					grid->elements.push_back(elem);
				}
			}
		} else {
			std::cerr << "Unhandled Line: " << line << std::endl;
		}
	}
	return grid;
}
