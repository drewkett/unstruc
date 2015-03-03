#include "su2.h"

#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

bool toSU2(const std::string& outputfile, const Grid& grid) {
	int i, j;

	FILE * f;
	f = fopen(outputfile.c_str(),"w");
	if (!f) Fatal("Could not open file");
	std::cerr << "Outputting SU2" << std::endl;
	std::cerr << "Writing Elements" << std::endl;
	fprintf(f,"NDIME= %d\n\n",grid.dim);
	int n_volume_elements = 0;
	for (const Element& e : grid.elements)
		if (Shape::Info[e.type].dim == grid.dim)
			n_volume_elements++;

	fprintf(f,"NELEM= %d\n",n_volume_elements);
	for (const Element& e : grid.elements) {
		if (Shape::Info[e.type].dim != grid.dim) continue;
		fprintf(f,"%d",Shape::Info[e.type].vtk_id);
		for (int p : e.points) {
			assert (p < grid.points.size());
			fprintf(f," %d",p);
		}
		fprintf(f,"\n");
	}
	fprintf(f,"\n");
	std::cerr << "Writing Points" << std::endl;
	fprintf(f,"NPOIN= %zu\n",grid.points.size());
	for (i = 0; i < grid.points.size(); i++) {
		const Point& p = grid.points[i];
		if (grid.dim == 2)
			fprintf(f,"%.17g %.17g %d\n",p.x,p.y,i);
		else
			fprintf(f,"%.17g %.17g %.17g %d\n",p.x,p.y,p.z,i);
	}
	fprintf(f,"\n");
	std::vector<int> name_count(grid.names.size(),0);
	for (i = 0; i < grid.elements.size(); i++) {
		const Element &e = grid.elements[i];
		if (Shape::Info[e.type].dim != (grid.dim-1)) continue;
		if (e.name_i < 0) continue;
		name_count[e.name_i]++;
	}
	int n_names = 0;
	for (i = 0; i < grid.names.size(); i++) {
		if (name_count[i] > 0)
			n_names++;
	}
	std::cerr << "Writing Markers" << std::endl;
	fprintf(f,"NMARK= %d\n",n_names);
	for (i = 0; i < grid.names.size(); i++) {
		if (name_count[i] == 0) continue;
		const Name& name = grid.names[i];
		if (name.dim != grid.dim - 1) continue;
		std::cerr << i << " : " << name.name << " (" << name_count[i] << ")" << std::endl;
		fprintf(f,"MARKER_TAG= %s\n",name.name.c_str());
		fprintf(f,"MARKER_ELEMS= %d\n",name_count[i]);
		for (j = 0; j < grid.elements.size(); j++) {
			const Element &e = grid.elements[j];
			if (Shape::Info[e.type].dim != grid.dim - 1) continue;
			if (e.name_i != i) continue;
			fprintf(f,"%d",Shape::Info[e.type].vtk_id);
			for (int p : e.points) {
				assert (p < grid.points.size());
				fprintf(f," %d",p);
			}
			fprintf(f,"\n");
		}
	}
	fprintf(f,"\n");
	fclose(f);
	return true;
}

Grid readSU2(const std::string& inputfile) {
	Grid grid;
	int ipoint, iname, i, j, k;
	int nelem;
	Name name;
	std::ifstream f;
	std::istringstream ls;
	std::string line, token;
	std::map<int,int> point_map;
	bool use_point_map = false;
	f.open(inputfile.c_str(),std::ios::in);
	std::cerr << "Opening SU2 File '" << inputfile << "'" << std::endl;
	if (!f.is_open()) Fatal("Could not open file");
	bool read_dime = false, read_poin = false, read_elem = false;
	while (getline(f,line)) {
		std::stringstream ss(line);
		ss >> token;
		if (token.substr(0,6) == "NDIME=") {
			read_dime = true;
			if (token.size() > 6) {
				grid.dim = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				grid.dim = std::atoi(token.c_str());
			}
			std::cerr << grid.dim << " Dimensions" << std::endl;

			//Create default named block to be assigned to all elements
			name = Name();
			name.name.assign("default");
			name.dim = grid.dim;
			grid.names.push_back(name);
		} else if (token.substr(0,6) == "NELEM=") {
			read_elem = true;
			int n_elems = 0;
			if (token.size() > 6) {
				n_elems = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				n_elems = std::atoi(token.c_str());
			}
			std::cerr << n_elems << " Elements" << std::endl;
			grid.elements.reserve(n_elems);
			for (int i = 0; i < n_elems; ++i) {
				getline(f,line);
				std::stringstream ss(line);
				ss >> token;
				Shape::Type type = type_from_vtk_id(atoi(token.c_str()));
				if (type == Shape::Undefined) Fatal("Unrecognized shape type");
				Element elem = Element(type);
				//Assign to default name block
				elem.name_i = 0;
				for (j = 0; j < elem.points.size(); ++j) {
					ss >> token;
					ipoint = std::atoi(token.c_str());
					elem.points[j] = ipoint;
				}
				grid.elements.push_back(elem);
			}
		} else if (token.substr(0,6) == "NPOIN=") {
			read_poin = true;
			int n_points = 0;
			if (token.size() > 6) {
				n_points = std::atoi(token.substr(6).c_str());
			} else {
				ss >> token;
				n_points = std::atoi(token.c_str());
			}
			grid.points.resize(n_points);
			if (ss >> token && !ss.eof())
				n_points = std::atoi(token.c_str());

			if (n_points > grid.points.size())
				grid.points.resize(n_points);

			std::cerr << n_points << " Points" << std::endl;

			for (i = 0; i < n_points; ++i) {
				getline(f,line);
				std::stringstream ss(line);
				Point point;
				ss >> token;
				point.x = std::atof(token.c_str());
				ss >> token;
				point.y = std::atof(token.c_str());
				if (!grid.dim)
					Fatal("Dimension (NDIME) not defined");
				if (grid.dim == 3) {
					ss >> token;
					point.z = std::atof(token.c_str());
				}
				if (!ss.eof()) {
					ss >> token;
					ipoint = std::atoi(token.c_str());
					use_point_map = true;
				} else {
					ipoint = i;
				}
				point_map[ipoint] = i;
				grid.points[i] = point;
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
				name = Name();
				name.dim = grid.dim-1;
				iname = grid.names.size();

				// Read MARKER_TAG=
				getline(f,line);
				std::stringstream ss(line);
				ss >> token;
				if (token.substr(0,11) != "MARKER_TAG=")
					Fatal("Invalid Marker Definition: Expected MARKER_TAG=");
				if (token.size() > 11) {
					name.name.assign(token.substr(11));
				} else {
					ss >> token;
					name.name.assign(token);
				}
				grid.names.push_back(name);
				std::cerr << name.name << std::endl;

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
					Shape::Type type = type_from_vtk_id(atoi(token.c_str()));
					if (type == Shape::Undefined) Fatal("Unrecognized shape type");
					Element elem = Element(type);
					for (k=0; k<elem.points.size(); k++) {
						ss >> token;
						ipoint = std::atoi(token.c_str());
						if (ipoint >= grid.points.size()) Fatal("Error Marker Element");
						elem.points[k] = ipoint;
					}
					elem.name_i = iname;
					grid.elements.push_back(elem);
				}
			}
		} 
	}
	assert (read_dime);
	assert (read_elem);
	assert (read_poin);
	int n_negative = 0;
	for (Element& e : grid.elements) {
		if (use_point_map) {
			for (int& p : e.points) {
				assert (point_map.count(p) == 1);
				p = point_map[p];
			}
		}
		if (e.calc_volume(grid) < 0) {
			if (e.type == Shape::Tetra) {
				std::swap(e.points[1],e.points[2]);
				assert (e.calc_volume(grid) > 0);
			}
			n_negative++;
		}
	}
	if (n_negative)
		fprintf(stderr,"%d Negative Volume Elements\n",n_negative);
	return grid;
}
