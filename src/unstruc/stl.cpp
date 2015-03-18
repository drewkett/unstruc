
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

namespace unstruc {

Point stl_read_vertex_ascii(std::istream& ss) {
	Point p;
	std::string token;
	ss >> token;
	if (token != "vertex") fatal("Expected vertex");
	ss >> p.x;
	ss >> p.y;
	ss >> p.z;
	return p;
}

Grid stl_read_ascii(const std::string& filename) {
	fprintf(stderr,"Reading ASCII STL File '%s'\n",filename.c_str());
	Grid grid (3);
	grid.names.push_back( Name(2,filename) );
	std::ifstream f;
	f.open(filename);
	if (!f.is_open()) fatal("Could not open file");
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
			if (token != "normal") fatal("Expected normal after facet");
			Vector normal;
			f >> normal.x;
			f >> normal.y;
			f >> normal.z;

			f >> token;
			if (token != "outer") fatal("Expected outer after normal definition");
			f >> token;
			if (token != "loop") fatal("Expected loop after outer");
			
			int i = grid.points.size();
			grid.points.push_back(stl_read_vertex_ascii(f));
			grid.points.push_back(stl_read_vertex_ascii(f));
			grid.points.push_back(stl_read_vertex_ascii(f));

			Element e (Shape::Triangle);
			e.points[0] = i;
			e.points[1] = i+1;
			e.points[2] = i+2;
			e.name_i = 1;
			grid.elements.push_back(e);

			f >> token;
			if (token != "endloop") fatal("Expected endloop");
			f >> token;
			if (token != "endfacet") fatal("Expected endfacet");
		} else {
			char c_msg[100];
			snprintf(c_msg,100,"Unknown Token '%s'",token.c_str());
			fatal(std::string(c_msg));
		}
	}
	fprintf(stderr,"%lu Triangles Read\n",grid.elements.size());
	return grid;
}

Point stl_read_vertex_binary(std::ifstream& f) {
	float x,y,z;
	f.read((char *) &x, sizeof(x));
	f.read((char *) &y, sizeof(y));
	f.read((char *) &z, sizeof(z));
	return Point {x,y,z};
}

Grid stl_read_binary(const std::string& filename) {
	fprintf(stderr,"Reading Binary STL File '%s'\n",filename.c_str());
	Grid grid (3);
	grid.names.push_back( Name(2,filename) );
	std::ifstream f;
	f.open(filename);
	if (!f.is_open()) fatal("Could not open file");
	char header[80];

	f.read(header, sizeof(header));
	uint32_t n_triangles;
	f.read((char *) &n_triangles, sizeof(n_triangles));
	fprintf(stderr,"Reading %d Triangles\n",n_triangles);

	for (int i = 0; i < n_triangles; ++i) {
		Point normal = stl_read_vertex_binary(f);
		grid.points.push_back(stl_read_vertex_binary(f));
		grid.points.push_back(stl_read_vertex_binary(f));
		grid.points.push_back(stl_read_vertex_binary(f));
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
		fatal("(read_stl_binary) End Of File not reached");
	return grid;
}

Grid stl_read(const std::string& filename) {
	std::ifstream f;
	f.open(filename);
	if (!f.is_open()) fatal("Could not open file");
	std::string token;
	f >> token;
	if (token == "solid") {
		f >> token;
		if (token == "facet")
			return stl_read_ascii(filename);
		f >> token;
		if (token == "facet")
			return stl_read_ascii(filename);
	}
	return stl_read_binary(filename);
}

void stl_write_ascii(const std::string& filename, const Grid& grid) {
	FILE * f;
	f = fopen(filename.c_str(),"w");
	fprintf(stderr,"Writing %s\n",filename.c_str());
	if (!f) fatal("Could not open file");
	for (const Element& e : grid.elements) {
		if (e.type != Shape::Triangle)
			fatal("STL files only support triangles");
		if (e.points.size() != 3)
			fatal("One of the triangles does not have 3 points");
	}
	fprintf(f,"solid\n");
	for (const Element& e : grid.elements) {
		fprintf(f,"  facet normal 0.0 0.0 0.0\n");
		fprintf(f,"    outer loop\n");
		for (int _p : e.points) {
			const Point& p = grid.points[_p];
			fprintf(f,"      vertex %.17g %.17g %.17g\n",p.x,p.y,p.z);
		}
		fprintf(f,"    endloop\n");
		fprintf(f,"  endfacet\n");
	}
	fprintf(f,"endsolid\n");
}

void stl_write_binary_vertex(FILE * f, const Point& p) {
	float x = p.x;
	fwrite(&x,sizeof(x),1,f);
	float y = p.y;
	fwrite(&y,sizeof(y),1,f);
	float z = p.z;
	fwrite(&z,sizeof(z),1,f);
}

void stl_write_binary_vertex(FILE * f, const Vector& v) {
	float x = v.x;
	fwrite(&x,sizeof(x),1,f);
	float y = v.y;
	fwrite(&y,sizeof(y),1,f);
	float z = v.z;
	fwrite(&z,sizeof(z),1,f);
}

void stl_write_binary(const std::string& filename, const Grid& grid) {
	FILE * f;
	f = fopen(filename.c_str(),"w");
	fprintf(stderr,"Writing %s\n",filename.c_str());
	if (!f) fatal("Could not open file");
	for (const Element& e : grid.elements) {
		if (e.type != Shape::Triangle)
			fatal("STL files only support triangles");
		if (e.points.size() != 3)
			fatal("One of the triangles does not have 3 points");
	}
	char header[80];
	fwrite(header,sizeof(header),1,f);
	uint32_t n = grid.elements.size();
	fwrite(&n,sizeof(n),1,f);
	for (const Element& e : grid.elements) {
		Vector normal {0, 0, 1};
		stl_write_binary_vertex(f,normal);
		for (int _p : e.points) {
			const Point& p = grid.points[_p];
			stl_write_binary_vertex(f,p);
		}
		uint16_t attr = 0;
		fwrite(&attr,sizeof(attr),1,f);
	}
}

} // namespace unstruc::stl
