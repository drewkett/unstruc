
#include "stl.h"
#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <cfloat>
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
			std::streamoff curr = f.tellg();
			f >> solid_name;
			if (solid_name == "facet") {
				solid_name.clear();
				f.seekg(curr);
			}
			in_solid = true;
			continue;
		} else if (token == "endsolid") {
			std::streamoff curr = f.tellg();
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
			
			size_t i = grid.points.size();
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

uint16_t read_uint16(std::ifstream& f) {
	uint16_t v;
	f.read(reinterpret_cast<char *>(&v), sizeof(v));
	return v;
}

uint32_t read_uint32(std::ifstream& f) {
	uint32_t v;
	f.read(reinterpret_cast<char *>(&v), sizeof(v));
	return v;
}

float read_float(std::ifstream& f) {
	float v;
	f.read(reinterpret_cast<char *>(&v), sizeof(v));
	return v;
}

Point stl_read_vertex_binary(std::ifstream& f) {
	float x = read_float(f);
	float y = read_float(f);
	float z = read_float(f);
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
	uint32_t n_triangles = read_uint32(f);
	fprintf(stderr,"Reading %d Triangles\n",n_triangles);

	for (size_t i = 0; i < n_triangles; ++i) {
		Point normal = stl_read_vertex_binary(f);
		grid.points.push_back(stl_read_vertex_binary(f));
		grid.points.push_back(stl_read_vertex_binary(f));
		grid.points.push_back(stl_read_vertex_binary(f));
		uint16_t attr = read_uint16(f);

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
	std::ofstream f (filename, std::ofstream::out);
	std::cerr << "Writing " << filename << std::endl;
	if (!f.is_open()) fatal("Could not open file");
	for (const Element& e : grid.elements) {
		if (e.type != Shape::Triangle)
			fatal("STL files only support triangles");
		if (e.points.size() != 3)
			fatal("One of the triangles does not have 3 points");
	}
	f.precision(DBL_DIG);
	f << "solid" << std::endl;
	for (const Element& e : grid.elements) {
		f << "  facet normal 0.0 0.0 0.0" << std::endl;
		f << "    outer loop" << std::endl;
		for (size_t _p : e.points) {
			const Point& p = grid.points[_p];
			f << "      vertex " << p.x << " " << p.y << " " << p.z << std::endl;
		}
		f << "    endloop" << std::endl;
		f << "  endfacet" << std::endl;
	}
	f << "endsolid" << std::endl;
}

void write_float(std::ofstream& f, float v) {
	f.write(reinterpret_cast <char *> (&v),sizeof(v));
}

void write_uint32(std::ofstream& f, uint32_t v) {
	f.write(reinterpret_cast <char *> (&v),sizeof(v));
}

void write_uint16(std::ofstream& f, uint16_t v) {
	f.write(reinterpret_cast <char *> (&v),sizeof(v));
}

void stl_write_binary_vertex(std::ofstream& f, const Point& p) {
	write_float(f,float(p.x));
	write_float(f,float(p.y));
	write_float(f,float(p.z));
}

void stl_write_binary_vertex(std::ofstream& f, const Vector& v) {
	write_float(f,float(v.x));
	write_float(f,float(v.y));
	write_float(f,float(v.z));
}

void stl_write_binary(const std::string& filename, const Grid& grid) {
	std::ofstream f (filename, std::ofstream::out | std::ofstream::binary);
	std::cerr << "Writing " << filename << std::endl;
	if (!f.is_open()) fatal("Could not open file");
	for (const Element& e : grid.elements) {
		if (e.type != Shape::Triangle)
			fatal("STL files only support triangles");
		if (e.points.size() != 3)
			fatal("One of the triangles does not have 3 points");
	}
	std::string header (80,' ');
	f << header;
	write_uint32(f,grid.elements.size());
	for (const Element& e : grid.elements) {
		Vector normal {0, 0, 1};
		stl_write_binary_vertex(f,normal);
		for (size_t _p : e.points) {
			const Point& p = grid.points[_p];
			stl_write_binary_vertex(f,p);
		}
		write_uint16(f,0);
	}
}

} // namespace unstruc::stl
