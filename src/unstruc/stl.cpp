
#include "stl.h"
#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <cstdio>

Point read_vertex_ascii(std::istream& ss) {
	Point p;
	std::string token;
	ss >> token;
	if (token != "vertex") Fatal("Expected vertex");
	ss >> p.x;
	ss >> p.y;
	ss >> p.z;
	return p;
}

Grid read_stl_ascii(const std::string& filename) {
	fprintf(stderr,"Reading ASCII STL File '%s'\n",filename.c_str());
	Grid grid (3);
	grid.names.emplace_back(2,filename);
	std::ifstream f;
	f.open(filename);
	bool in_solid = false;
	std::string token;
	std::string solid_name;
	while (f >> token) {
		if (token == "solid") {
			int curr = f.tellg();
			f >> solid_name;
			if (solid_name == "facet") {
				solid_name.clear();
				f.seekg(curr);
			}
			in_solid = true;
			continue;
		} else if (token == "endsolid") {
			int curr = f.tellg();
			std::string temp;
			f >> temp;
			if (temp != solid_name) {
				f.seekg(curr);
			}
			in_solid = false;
			continue;
		} else if (in_solid && token == "facet") {
			f >> token;
			if (token != "normal") Fatal("Expected normal after facet");
			Vector normal;
			f >> normal.x;
			f >> normal.y;
			f >> normal.z;

			f >> token;
			if (token != "outer") Fatal("Expected outer after normal definition");
			f >> token;
			if (token != "loop") Fatal("Expected loop after outer");
			
			int i = grid.points.size();
			grid.points.push_back(read_vertex_ascii(f));
			grid.points.push_back(read_vertex_ascii(f));
			grid.points.push_back(read_vertex_ascii(f));

			Element e (Shape::Triangle);
			e.points[0] = i;
			e.points[1] = i+1;
			e.points[2] = i+2;
			e.name_i = 1;
			grid.elements.push_back(e);

			f >> token;
			if (token != "endloop") Fatal("Expected endloop");
			f >> token;
			if (token != "endfacet") Fatal("Expected endfacet");
		} else {
			char c_msg[100];
			snprintf(c_msg,100,"Unknown Token '%s'",token.c_str());
			Fatal(std::string(c_msg));
		}
	}
	fprintf(stderr,"%lu Triangles Read\n",grid.elements.size());
	return grid;
}

Point read_vertex_binary(std::ifstream& f) {
	float x,y,z;
	f.read((char *) &x, sizeof(x));
	f.read((char *) &y, sizeof(y));
	f.read((char *) &z, sizeof(z));
	return Point (x,y,z);
}

Grid read_stl_binary(const std::string& filename) {
	fprintf(stderr,"Reading Binary STL File '%s'\n",filename.c_str());
	Grid grid (3);
	grid.names.emplace_back(2,filename);
	std::ifstream f;
	f.open(filename);
	char header[80];

	f.read(header, sizeof(header));
	uint32_t n_triangles;
	f.read((char *) &n_triangles, sizeof(n_triangles));
	fprintf(stderr,"Reading %d Triangles\n",n_triangles);

	for (int i = 0; i < n_triangles; ++i) {
		Point normal = read_vertex_binary(f);
		grid.points.push_back(read_vertex_binary(f));
		grid.points.push_back(read_vertex_binary(f));
		grid.points.push_back(read_vertex_binary(f));
		uint16_t attr;
		f.read((char *) &attr,sizeof(attr));

		Element e (Shape::Triangle);
		e.points[0] = 3*i;
		e.points[1] = 3*i+1;
		e.points[2] = 3*i+2;
		e.name_i = 1;
		grid.elements.push_back(e);
	}
	f.get();
	if (!f.eof())
		Fatal("(read_stl_binary) End Of File not reached");
	return grid;
}

Grid readSTL(const std::string& filename) {
	std::ifstream f;
	f.open(filename);
	std::string token;
	f >> token;
	if (token == "solid")
		return read_stl_ascii(filename);
	else
		return read_stl_binary(filename);
}
