
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

Grid read_stl_ascii(std::string filename) {
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

			Element e (TRI);
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
	return grid;
}
