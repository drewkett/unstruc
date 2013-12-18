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

bool toSU2(std::string * outputfile, Grid * grid) {
	int i, j;

	std::fstream f;
	f.open(outputfile->c_str(),std::ios::out);
	if (!f.is_open()) Fatal("Could not open file");
	f.precision(15);
	Name * name;
	Point * p;
	Element * e;
	set_i(grid);
	sort_elements_by_name(grid);
	std::cerr << "Outputting SU2" << std::endl;
	std::cerr << "Writing Elements" << std::endl;
	f << "NDIME= " << 3 << std::endl;
	f << std::endl;
	f << "NELEM= " << grid->n_elems << std::endl;
	for (i = 0; i < grid->elements.size(); i++) {
		e = grid->elements[i];
		if (!e or e->dim != 3) continue;
		f << e->type;
		for (j = 0; j < e->len; j++) {
			f << " " << (**e->points[j]).i;
		}
		f << " " << i;
		f << std::endl;
	}
	f << std::endl;
	std::cerr << "Writing Points" << std::endl;
	f << "NPOIN= " << grid->n_points << std::endl;
	for (i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		p = *grid->ppoints[i];
		f << p->x << " " << p->y << " " << p->z << " " << p->i << std::endl;
	}
	f << std::endl;
	int n_names = 0;
	for (i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		if (grid->names[i]->dim != 2) continue;
		n_names++;
	}
	int* name_count = new int[grid->names.size()];
	for (i = 0; i < grid->names.size(); i++) {
		name_count[i] = 0;
	}
	for (i = 0; i < grid->elements.size(); i++) {
		e = grid->elements[i];
		if (!e or e->dim != 2) continue;
		name_count[e->name_i]++;
	}
	std::cerr << "Writing Markers" << std::endl;
	f << "NMARK= " << n_names << std::endl;
	for (i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		name = grid->names[i];
		if (name->dim != 2) continue;
		std::cerr << i << " : " << name->name << std::endl;
		f << "MARKER_TAG= " << name->name << std::endl;
		f << "MARKER_ELEMS= " << name_count[i] << std::endl;
		for (j = 0; j < grid->elements.size(); j++) {
			e = grid->elements[j];
			if (!e or e->dim != 2) continue;
			if (e->name_i != i) continue;
			f << e->type;
			for (int k = 0; k < e->len; k++) {
				f << " " << (**e->points[k]).i;
			}
			f << std::endl;
		}
	}
	return true;
}

Grid * readSU2(std::string * inputfile) {
	int ipoint, ielem, iname, i, j, k;
	int npoint, nelem, nmark;
	Grid * grid = new Grid();
	Element * elem;
	Point * point;
	Name * name;
	std::ifstream f;
	std::istringstream ls;
	std::string line, token;
	f.open(inputfile->c_str(),std::ios::in);
	std::cerr << "Opening SU2 File '" << *inputfile << "'" << std::endl;
	if (!f.is_open()) Fatal("Could not open file");
	while (f >> token) {
		if (token == "NDIME=") {
			f >> token;
			grid->dim = std::atoi(token.c_str());
			break;
		}
	}
	while (f >> token) {
		if (token == "NELEM=") {
			f >> token;
			grid->n_elems = atoi(token.c_str());
			grid->elements.resize(grid->n_elems);
			break;
		}
	}
	for (i=0; i<grid->n_elems; i++) {
		f >> token;
		elem = new Element(atoi(token.c_str()));
		for (j=0; j<elem->len; j++) {
			f >> token;
			ipoint = std::atoi(token.c_str());
			if (ipoint >= grid->points.size()) {
				grid->points.resize(ipoint+1);
				grid->ppoints.resize(ipoint+1);
			}
			elem->points[j] = &grid->points[ipoint];
		}
		f >> token;
		ielem = atoi(token.c_str());
		if (ielem >= grid->elements.size())
			grid->elements.resize(ielem+1);
		grid->elements[ielem] = elem;
	}
	while (f >> token) {
		if (token == "NPOIN=") {
			getline(f,line);
			ls.str(line);
			ls >> token;
			npoint = atoi(token.c_str());
			if (ls >> token && !ls.eof()) {
				npoint = atoi(token.c_str());
			}
			grid->n_points = npoint;
			if (npoint > grid->points.size()) {
				grid->points.resize(npoint);
				grid->ppoints.resize(npoint);
			}
			break;
		}
	}
	for (i=0; i<grid->n_points; i++) {
		point = new Point();
		f >> token;
		point->x = std::atof(token.c_str());
		f >> token;
		point->y = std::atof(token.c_str());
		if (grid->dim == 3) {
			f >> token;
			point->z = std::atof(token.c_str());
		}
		f >> token;
		ipoint = std::atoi(token.c_str());
		point->i = ipoint;
		grid->points[ipoint] = point;
		grid->ppoints[ipoint] = &grid->points[ipoint];
	}
	while (f >> token) {
		if (token == "NMARK=") {
			f >> token;
			nmark = atoi(token.c_str());
			break;
		}
	}
	for (i = 0; i < nmark; i++) {
		name = new Name();
		name->dim = grid->dim-1;
		iname = grid->names.size();
		grid->names.push_back(name);
		while (f >> token) {
			if (token == "MARKER_TAG=") {
				getline(f,line);
				ls.clear();
				ls.str(line);
				ls >> token;
				name->name = token;
				break;
			}
		}
		while (f >> token) {
			if (token == "MARKER_ELEMS=") {
				getline(f,line);
				ls.clear();
				ls.str(line);
				ls >> token;
				nelem = atoi(token.c_str());
				break;
			}
		}
		for (j=0; j < nelem; j++) {
			getline(f,line);
			ls.clear();
			ls.str(line);
			ls >> token;
			elem = new Element(atoi(token.c_str()));
			for (k=0; k<elem->len; k++) {
				ls >> token;
				ipoint = std::atoi(token.c_str());
				if (ipoint >= grid->points.size()) 
					Fatal("Error Reading File");
				elem->points[k] = &grid->points[ipoint];
			}
			elem->name_i = iname;
			grid->elements[j] = elem;
		}
	}
	return grid;
}
