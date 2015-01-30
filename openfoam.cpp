
#include "openfoam.h"
#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

enum OFCellType {
	OFUnknown,
	OFHexa,
	OFWedge,
	OFPrism,
	OFPyramid,
	OFTetra,
	OFTetraWedge,
	OFPoly
};

struct FoamHeader {
	std::string version;
	std::string format;
	std::string _class;
	std::string location;
	std::string object;
	std::string note;
};

struct OFPoint {
	double x, y, z;
};

struct OFBoundary {
	std::string name;
	int n_faces;
	int start_face;
};

struct OFFace {
	int n_points;
	std::vector<int> points;
};

struct OFInfo {
	int n_points, n_cells, n_faces, n_internal_faces;
};

std::vector<OFFace> splitPolyFace(OFFace& face, Grid& grid) {
	if (face.n_points < 5) Fatal("(openfoam.cpp::splitFace) Not a PolyFace");
	std::vector<OFFace> split_faces;

	Point* face_center = new Point { 0, 0, 0, 0, 0 };
	for (int p_i : face.points) {
		Point *p = grid.points[p_i];
		face_center->x += p->x;
		face_center->y += p->y;
		face_center->z += p->z;
	}
	face_center->x /= face.n_points;
	face_center->y /= face.n_points;
	face_center->z /= face.n_points;

	int face_center_id = grid.points.size();
	grid.points.push_back(face_center);
	grid.ppoints.push_back(&grid.points.back());

	//create tetrahedrals that include one edge, the face center and the cell center
	//for (int k = current_index; k < current_index+current_npoints-1; ++k) {
	for (int i = 0; i < face.n_points-1; ++i) {
		split_faces.emplace_back();
		OFFace& new_face = split_faces.back();
		new_face.n_points = 3;
		new_face.points.resize(3);
		new_face.points[0] = face.points[i];
		new_face.points[1] = face.points[i+1];
		new_face.points[2] = face_center_id;
	}
	return split_faces;
}

FoamHeader readFoamHeader(std::ifstream& f) {
	std::string line;
	bool is_foamfile = false;
	while (std::getline(f,line)) {
		if (line.compare(0,8,"FoamFile") == 0) {
			is_foamfile = true;
			break;
		}
	}
	if (!is_foamfile) Fatal("Not a valid FoamFile");

	std::string token;
	f >> token;
	if (token != "{") Fatal("Not a valid FoamFile");

	FoamHeader header;
	f >> token;
	while (token != "}") {
		if (token == "version") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.version = token;
		} else if (token == "format") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.format = token;
		} else if (token == "class") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header._class = token;
		} else if (token == "location") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.location = token;
		} else if (token == "object") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.object = token;
		} else if (token == "note") {
			std::getline(f,token,';');
			token.erase(0,token.find_first_not_of(' '));
			header.note = token;
		} else {
			std::cerr << token << std::endl;
			Fatal("Not a valid FoamFile");
		}
		f >> token;
	}

	return header;
}

int readNextInt(std::ifstream& f) {
	int n = -1;
	std::string line;
	while (std::getline(f,line)) {
		if (line.size() == 0) continue;
		if (line[0] == '/' && line[1] == '/') continue;
		n = atoi(line.c_str());
		break;
	}
	if (n == -1) Fatal("Invalid FoamFile");
	return n;
}

template<typename T>
std::vector<T> readBinary(std::ifstream& f, FoamHeader& header) {
	int n = readNextInt(f);

	char c = f.get();
	if (c != '(') Fatal("Invalid FoamFile: Expected '(' in "+header.location);

	std::vector<T> vec;
	vec.resize(n);
	for (int i = 0; i < n; ++i) {
		f.read((char *) &vec[i], sizeof(T));
	}
	
	c = f.get();
	if (c != ')') Fatal("Invalid FoamFile: Expected ')' in "+header.location);
	return vec;
}

OFInfo readInfoFromOwners(std::string polymesh) {
	std::string filepath = polymesh + "/owner";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f);

	std::string note = header.note.substr(1,header.note.size()-2);
	std::istringstream ss(note);
	std::string token;
	std::string name;
	int i;

	OFInfo info = {-1, -1, -1, -1};

	while (ss >> token) {
		i = token.find_first_of(':');
		if (token.compare(0,i,"nPoints") == 0) {
			info.n_points = atoi(token.substr(i+1).c_str());
		} else if (token.compare(0,i,"nCells") == 0) {
			info.n_cells = atoi(token.substr(i+1).c_str());
		} else if (token.compare(0,i,"nFaces") == 0) {
			info.n_faces = atoi(token.substr(i+1).c_str());
		} else if (token.compare(0,i,"nInternalFaces") == 0) {
			info.n_internal_faces = atoi(token.substr(i+1).c_str());
		} else {
			Fatal("Invalid FoamFile: Unknown key in header.note "+token.substr(0,i));
		}
	}
	if (info.n_points == -1) Fatal("Invalid FoamFile: nPoints not found in owner.note");
	if (info.n_cells == -1) Fatal("Invalid FoamFile: nCells not found in owner.note");
	if (info.n_faces == -1) Fatal("Invalid FoamFile: nFaces not found in owner.note");
	if (info.n_internal_faces == -1) Fatal("Invalid FoamFile: nInternalFaces not found in owner.note");
	return info;
}

std::vector<int> readOwners(const std::string polymesh) {
	std::string filepath = polymesh + "/owner";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<int>(f,header);
}

std::vector<int> readNeighbours(const std::string& polymesh) {
	std::string filepath = polymesh + "/neighbour";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<int>(f,header);
}

std::vector<OFPoint> readPoints(const std::string& polymesh) {
	std::string filepath = polymesh + "/points";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<OFPoint>(f,header);
}

std::vector<OFFace> readFaces(const std::string& polymesh) {
	std::string filepath = polymesh + "/faces";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	std::vector<OFFace> faces;
	std::vector<int> index = readBinary<int>(f,header);
	std::vector<int> points = readBinary<int>(f,header);

	faces.resize(index.size() - 1);
	for (int i = 0; i < index.size() - 1; ++i) {
		faces[i].n_points = index[i+1] - index[i];
		faces[i].points = std::vector<int> (&points[index[i]],&points[index[i+1]]);
	}

	return faces;
}

std::vector<OFBoundary> readBoundaries(const std::string& polymesh) {
	std::string filepath = polymesh + "/boundary";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath);
	FoamHeader header = readFoamHeader(f);

	int n = readNextInt(f);

	std::string token;
	f >> token;
	if (token != "(") Fatal("Invalid FoamFile: Expected '(' in "+header.location);

	std::vector<OFBoundary> boundaries;
	boundaries.resize(n);

	for (int i = 0; i < n; ++i) {
		f >> boundaries[i].name;
		boundaries[i].n_faces = -1;
		boundaries[i].start_face = -1;
		f >> token;
		if (token != "{") Fatal("Invalid FoamFile: Expected '{' in "+header.location);
		while (token != "}") {
			if (token == "nFaces") {
				std::getline(f,token,';');
				token.erase(0,token.find_first_not_of(' '));
				boundaries[i].n_faces = atoi(token.c_str());
			} else if (token == "startFace") {
				std::getline(f,token,';');
				token.erase(0,token.find_first_not_of(' '));
				boundaries[i].start_face = atoi(token.c_str());
			}
			f >> token;
		}
		if (boundaries[i].n_faces == -1) Fatal("Invalid FoamFile: Expected nFaces for patch "+boundaries[i].name);
		if (boundaries[i].start_face == -1) Fatal("Invalid FoamFile: Expected startFace for patch "+boundaries[i].name);
	}

	f >> token;
	if (token != ")") Fatal("Invalid FoamFile: Expected ')' in "+header.location);

	return boundaries;
}

void readOpenFoam(Grid& grid, std::string &polymesh) {
	struct stat s;
	if (! (stat(polymesh.c_str(),&s) == 0 && (s.st_mode & S_IFDIR)) )
		Fatal(polymesh + " isn't a directory");
	
	OFInfo info = readInfoFromOwners(polymesh);
	std::vector<OFPoint> points = readPoints(polymesh);
	std::vector<OFFace> faces = readFaces(polymesh);
	std::vector<int> owners = readOwners(polymesh);
	std::vector<int> neighbours = readNeighbours(polymesh);
	std::vector<OFBoundary> boundaries = readBoundaries(polymesh);

	printf("Points: %d\nFaces: %d\nInternal Faces: %d\nCells: %d\n",info.n_points,info.n_faces,info.n_internal_faces,info.n_cells);
	if (info.n_points != points.size()) Fatal("Invalid FoamFile: number of points do not match");
	grid.points.resize(points.size());
	grid.ppoints.resize(points.size());
	for (int i = 0; i < points.size(); ++i) {
		Point* p = new Point;
		p->x = points[i].x;
		p->y = points[i].y;
		p->z = points[i].z;
		grid.points[i] = p;
		grid.ppoints[i] = &grid.points[i];
	}

	if (info.n_faces != faces.size() ) Fatal("Invalid FoamFile: number of faces do not match");

	std::vector< int > n_faces_per_cell (info.n_cells,0);
	std::vector< int > n_owners_per_cell (info.n_cells,0);
	std::vector< std::vector<int> > faces_per_cell (info.n_cells);
	for (int i = 0; i < owners.size(); ++i) {
		n_faces_per_cell[owners[i]]++;
		n_owners_per_cell[owners[i]]++;
		faces_per_cell[owners[i]].push_back(i);
	}
	for (int i = 0; i < neighbours.size(); ++i) {
		n_faces_per_cell[neighbours[i]]++;
		faces_per_cell[neighbours[i]].push_back(i);
	}
	for (int i = 0; i < info.n_cells; ++i) {
		OFCellType cell_type = OFUnknown;
		int n_tri = 0;
		int n_quad = 0;
		int n_poly = 0;
		std::vector<int>& cell_faces = faces_per_cell[i];
		for (int j : cell_faces) {
			OFFace& current_face = faces[j];
			switch (current_face.n_points) {
				case 0:
				case 1:
				case 2:
					Fatal("Bad Face");
				case 3:
					n_tri++;
					break;
				case 4:
					n_quad++;
					break;
				default:
					n_poly++;
					break;
			}
		}
		switch (n_faces_per_cell[i]) {
			case 0:
			case 1:
			case 2:
			case 3:
				Fatal("Bad Cell");
			case 4:
				if (n_poly)
					cell_type = OFPoly;
				else if (n_tri == 4)
					cell_type = OFTetra;
				else if (n_quad == 2 && n_tri == 2)
					cell_type = OFTetraWedge;
				break;
			case 5:
				if (n_poly)
					cell_type = OFPoly;
				else if (n_quad == 3 && n_tri == 2)
					cell_type = OFPrism;
				else if (n_quad == 1 && n_tri == 4)
					cell_type = OFPyramid;
				break;
			case 6:
				if (n_poly)
					cell_type = OFPoly;
				else if (n_quad == 6)
					cell_type = OFHexa;
				else if (n_quad == 4 && n_tri == 2)
					cell_type = OFWedge;
				break;
			default:
				cell_type = OFPoly;
				break;
		}
		if (cell_type == OFUnknown) {
			printf("Cell %d nFaces %d nTri %d nQuad %d\n",i,n_faces_per_cell[i],n_tri,n_quad);
			Fatal("Unknown Cell Type");
		} else if (cell_type == OFTetra) {
			grid.elements.emplace_back(TETRA);
			Element& e = grid.elements.back();
			std::vector<int>& cell_faces = faces_per_cell[i];
			int first_face = cell_faces[0];
			if (n_owners_per_cell[i] == 0) {
				e.points[0] = &grid.points[faces[first_face].points[0]];
				e.points[1] = &grid.points[faces[first_face].points[1]];
				e.points[2] = &grid.points[faces[first_face].points[2]];
			} else {
				e.points[2] = &grid.points[faces[first_face].points[0]];
				e.points[1] = &grid.points[faces[first_face].points[1]];
				e.points[0] = &grid.points[faces[first_face].points[2]];
			}
			int second_face = cell_faces[1];
			for (int p2 : faces[second_face].points) {
				bool match = true;
				for (int p1 : faces[first_face].points) {
					if (p2 == p1) {
						match = false;
						break;
					}
				}
				if (match) {
					e.points[3] = &grid.points[p2];
					break;
				}
			}
		} else if (cell_type == OFTetraWedge) {
			std::vector<int>& cell_faces = faces_per_cell[i];
			int tri1_j = -1;
			int tri2_j = -1;
			int quad1_j = -1;
			for (int j = 0; j < 5; ++j) {
				if (faces[cell_faces[j]].n_points == 3) {
					if (tri1_j == -1)
						tri1_j = j;
					else
						tri2_j = j;
				} else {
					if (quad1_j == -1)
						quad1_j = j;
				}
			}
			OFFace& tri1_face = faces[cell_faces[tri1_j]];
			OFFace& tri2_face = faces[cell_faces[tri2_j]];
			OFFace& quad1_face = faces[cell_faces[quad1_j]];

			int extra_point = -1;
			for (int p : quad1_face.points) {
				bool match = true;
				for (int tp1 : tri1_face.points) {
					if (p == tp1) {
						match = false;
						break;
					}
				}
				if (!match) continue;
				for (int tp2 : tri2_face.points) {
					if (p == tp2) {
						match = false;
						break;
					}
				}
				if (match) {
					extra_point = p;
					break;
				}
			}
			if (extra_point == -1) Fatal("Shouldn't be possible");

			grid.elements.emplace_back(TETRA);
			Element& e1 = grid.elements.back();
			if (tri1_j < n_owners_per_cell[i]) {
				e1.points[2] = &grid.points[tri1_face.points[0]];
				e1.points[1] = &grid.points[tri1_face.points[1]];
				e1.points[0] = &grid.points[tri1_face.points[2]];
			} else {
				e1.points[0] = &grid.points[tri1_face.points[0]];
				e1.points[1] = &grid.points[tri1_face.points[1]];
				e1.points[2] = &grid.points[tri1_face.points[2]];
			}
			e1.points[3] = &grid.points[extra_point];

			grid.elements.emplace_back(TETRA);
			Element& e2 = grid.elements.back();
			if (tri2_j < n_owners_per_cell[i]) {
				e2.points[2] = &grid.points[tri2_face.points[0]];
				e2.points[1] = &grid.points[tri2_face.points[1]];
				e2.points[0] = &grid.points[tri2_face.points[2]];
			} else {
				e2.points[0] = &grid.points[tri2_face.points[0]];
				e2.points[1] = &grid.points[tri2_face.points[1]];
				e2.points[2] = &grid.points[tri2_face.points[2]];
			}
			e2.points[3] = &grid.points[extra_point];
		} else if (cell_type == OFPyramid) {
			grid.elements.emplace_back(PYRAMID);
			Element& e = grid.elements.back();
			std::vector<int>& cell_faces = faces_per_cell[i];
			int quad_j = -1;
			for (int j = 0; j < 5; ++j) {
				if (faces[cell_faces[j]].n_points == 4) {
					quad_j = j;
					break;
				}
			}
			if (quad_j == -1) Fatal("Shouldn't be possible");

			OFFace& quad_face = faces[cell_faces[quad_j]];
			if (quad_j < n_owners_per_cell[i]) {
				e.points[3] = &grid.points[quad_face.points[0]];
				e.points[2] = &grid.points[quad_face.points[1]];
				e.points[1] = &grid.points[quad_face.points[2]];
				e.points[0] = &grid.points[quad_face.points[3]];
			} else {
				e.points[0] = &grid.points[quad_face.points[0]];
				e.points[1] = &grid.points[quad_face.points[1]];
				e.points[2] = &grid.points[quad_face.points[2]];
				e.points[3] = &grid.points[quad_face.points[3]];
			}
			int second_j;
			if (quad_j == 0)
				second_j = 1;
			else
				second_j = 0;
			OFFace& second_face = faces[cell_faces[second_j]];
			for (int p : second_face.points) {
				bool match = true;
				for (int p2 : quad_face.points) {
					if (p == p2) {
						match = false;
						break;
					}
				}
				if (match) {
					e.points[4] = &grid.points[p];
					break;
				}
			}
		} else if (cell_type == OFWedge) {
			std::vector<int>& cell_faces = faces_per_cell[i];
			int tri1_j = -1;
			int tri2_j = -1;
			for (int j = 0; j < 6; ++j) {
				if (faces[cell_faces[j]].n_points == 3) {
					if (tri1_j == -1)
						tri1_j = j;
					else if (tri2_j == -1)
						tri2_j = j;
					else
						Fatal("Shouldn't be possible");
				}
			}
			if (tri1_j == -1) Fatal("Shouldn't be possible");
			if (tri2_j == -1) Fatal("Shouldn't be possible");
			OFFace& tri1_face = faces[cell_faces[tri1_j]];
			OFFace& tri2_face = faces[cell_faces[tri2_j]];

			int common_point = -1;
			for (int p1 : tri1_face.points) {
				for (int p2 : tri2_face.points) {
					if (p1 == p2) {
						common_point = p1;
						break;
					}
				}
				if (common_point != -1) break;
			}
			if (common_point == -1) Fatal("Shouldn't be possible");

			int quad1_j = -1;
			int quad2_j = -1;
			for (int j = 0; j < 6; ++j) {
				OFFace& current_face = faces[cell_faces[j]];
				if (current_face.n_points == 3) continue;
				bool match1 = false;
				bool match2 = false;
				for (int p : current_face.points) {
					for (int p1 : tri1_face.points) {
						if (p == p1) {
							match1 = true;
							break;
						}
					}
					for (int p2 : tri2_face.points) {
						if (p == p2) {
							match2 = true;
							break;
						}
					}
					if (match1 && match2) break;
				}
				if (!match1) quad1_j = j;
				if (!match2) quad2_j = j;
			}
			if (quad1_j == -1) Fatal("Shouldn't be possible");
			if (quad2_j == -1) Fatal("Shouldn't be possible");

			OFFace& quad1_face = faces[cell_faces[quad1_j]];
			OFFace& quad2_face = faces[cell_faces[quad2_j]];

			grid.elements.emplace_back(PYRAMID);
			Element& e1 = grid.elements.back();
			if (quad1_j < n_owners_per_cell[i]) {
				e1.points[3] = &grid.points[quad1_face.points[0]];
				e1.points[2] = &grid.points[quad1_face.points[1]];
				e1.points[1] = &grid.points[quad1_face.points[2]];
				e1.points[0] = &grid.points[quad1_face.points[3]];
			} else {
				e1.points[0] = &grid.points[quad1_face.points[0]];
				e1.points[1] = &grid.points[quad1_face.points[1]];
				e1.points[2] = &grid.points[quad1_face.points[2]];
				e1.points[3] = &grid.points[quad1_face.points[3]];
			}
			e1.points[4] = &grid.points[common_point];

			grid.elements.emplace_back(PYRAMID);
			Element& e2 = grid.elements.back();
			if (quad2_j < n_owners_per_cell[i]) {
				e2.points[3] = &grid.points[quad2_face.points[0]];
				e2.points[2] = &grid.points[quad2_face.points[1]];
				e2.points[1] = &grid.points[quad2_face.points[2]];
				e2.points[0] = &grid.points[quad2_face.points[3]];
			} else {
				e2.points[0] = &grid.points[quad2_face.points[0]];
				e2.points[1] = &grid.points[quad2_face.points[1]];
				e2.points[2] = &grid.points[quad2_face.points[2]];
				e2.points[3] = &grid.points[quad2_face.points[3]];
			}
			e2.points[4] = &grid.points[common_point];
		} else if (cell_type == OFPrism) {
			std::vector<int>& cell_faces = faces_per_cell[i];
			int tri1_j = -1;
			int tri2_j = -1;
			for (int j = 0; j < 6; ++j) {
				if (faces[cell_faces[j]].n_points == 3) {
					if (tri1_j == -1)
						tri1_j = j;
					else if (tri2_j == -1)
						tri2_j = j;
					else
						Fatal("Shouldn't be possible");
				}
			}
			if (tri1_j == -1) Fatal("Shouldn't be possible");
			if (tri2_j == -1) Fatal("Shouldn't be possible");
			OFFace& tri1_face = faces[cell_faces[tri1_j]];
			OFFace& tri2_face = faces[cell_faces[tri2_j]];

			grid.elements.emplace_back(WEDGE);
			Element& e = grid.elements.back();
			if (tri1_j < n_owners_per_cell[i]) {
				e.points[0] = &grid.points[tri1_face.points[0]];
				e.points[1] = &grid.points[tri1_face.points[1]];
				e.points[2] = &grid.points[tri1_face.points[2]];
			} else {
				e.points[2] = &grid.points[tri1_face.points[0]];
				e.points[1] = &grid.points[tri1_face.points[1]];
				e.points[0] = &grid.points[tri1_face.points[2]];
			}
			if (tri2_j < n_owners_per_cell[i]) {
				e.points[5] = &grid.points[tri2_face.points[0]];
				e.points[4] = &grid.points[tri2_face.points[1]];
				e.points[3] = &grid.points[tri2_face.points[2]];
			} else {
				e.points[3] = &grid.points[tri2_face.points[0]];
				e.points[4] = &grid.points[tri2_face.points[1]];
				e.points[5] = &grid.points[tri2_face.points[2]];
			}
		} else if (cell_type == OFHexa) {
			grid.elements.emplace_back(HEXA);
			Element& e = grid.elements.back();
			std::vector<int>& cell_faces = faces_per_cell[i];
			OFFace& first_face = faces[cell_faces[0]];
			if (n_owners_per_cell[i] == 0) {
				e.points[0] = &grid.points[first_face.points[0]];
				e.points[1] = &grid.points[first_face.points[1]];
				e.points[2] = &grid.points[first_face.points[2]];
				e.points[3] = &grid.points[first_face.points[3]];
			} else {
				e.points[3] = &grid.points[first_face.points[0]];
				e.points[2] = &grid.points[first_face.points[1]];
				e.points[1] = &grid.points[first_face.points[2]];
				e.points[0] = &grid.points[first_face.points[3]];
			}

			std::vector<int> first_face_points (first_face.points);
			std::sort(first_face_points.begin(),first_face_points.end());

			std::vector<int> current_face_points (4);
			for (int j = 1; j < 6; ++j) {
				OFFace& current_face = faces[cell_faces[j]];
				std::vector<int> current_face_points (current_face.points);
				std::sort(current_face_points.begin(),current_face_points.end());

				bool match = true;
				for (int k = 0; k < 4; ++k) {
					if (first_face_points[k] == current_face_points[k]) {
						match = false;
						break;
					}
				}

				if (match) {
					if (j < n_owners_per_cell[i]) {
						e.points[4] = &grid.points[current_face.points[0]];
						e.points[5] = &grid.points[current_face.points[1]];
						e.points[6] = &grid.points[current_face.points[2]];
						e.points[7] = &grid.points[current_face.points[3]];
					} else {
						e.points[7] = &grid.points[current_face.points[0]];
						e.points[6] = &grid.points[current_face.points[1]];
						e.points[5] = &grid.points[current_face.points[2]];
						e.points[4] = &grid.points[current_face.points[3]];
					}
					break;
				}
			}
		} else if (cell_type == OFPoly) {
			// Find complete set of points that make up cell by doing repeated unions
			std::vector<int> point_set (0);
			std::sort(point_set.begin(),point_set.end());

			for (int f : cell_faces) {
				OFFace& current_face = faces[f];

				// create vector of points for current face
				std::vector<int> current_set (current_face.points);
				std::sort(current_set.begin(),current_set.end());

				// copy point_set to temp_set so that final union goes back in point_set
				std::vector<int> temp_set (point_set);

				// add space for union to add new values to point_set
				point_set.resize(current_set.size() + temp_set.size());

				std::vector<int>::iterator it;
				it = std::set_union(temp_set.begin(),temp_set.end(),current_set.begin(),current_set.end(),point_set.begin());
				point_set.resize(it-point_set.begin());
			}

			// Calculate cell_center used for created cells
			Point* cell_center = new Point { 0, 0, 0, 0, 0 };
			for (int p_i : point_set) {
				Point *p = grid.points[p_i];
				cell_center->x += p->x;
				cell_center->y += p->y;
				cell_center->z += p->z;
			}
			cell_center->x /= point_set.size();
			cell_center->y /= point_set.size();
			cell_center->z /= point_set.size();

			// need to add new point to grid
			int cell_center_id = grid.points.size();
			grid.points.push_back(cell_center);
			grid.ppoints.push_back(&grid.points.back());

			for (int j = 0; j < cell_faces.size(); ++j) {
				OFFace& current_face = faces[cell_faces[j]];

				if (current_face.n_points == 3) {
					// If current face only has 3 points, create a tetrahedral with face plus cell center
					grid.elements.emplace_back(TETRA);
					Element& e = grid.elements.back();

					if (j < n_owners_per_cell[i]) {
						e.points[0] = &grid.points[current_face.points[0]];
						e.points[1] = &grid.points[current_face.points[1]];
						e.points[2] = &grid.points[current_face.points[2]];
					} else {
						e.points[2] = &grid.points[current_face.points[0]];
						e.points[1] = &grid.points[current_face.points[1]];
						e.points[0] = &grid.points[current_face.points[2]];
					}
					e.points[3] = &grid.points[cell_center_id];
				} else if (current_face.n_points == 4) {
					// If current face only has 4 points, create a pyramid with face plus cell center
					grid.elements.emplace_back(PYRAMID);
					Element& e = grid.elements.back();

					if (j < n_owners_per_cell[i]) {
						e.points[0] = &grid.points[current_face.points[0]];
						e.points[1] = &grid.points[current_face.points[1]];
						e.points[2] = &grid.points[current_face.points[2]];
						e.points[3] = &grid.points[current_face.points[3]];
					} else {
						e.points[3] = &grid.points[current_face.points[0]];
						e.points[2] = &grid.points[current_face.points[1]];
						e.points[1] = &grid.points[current_face.points[2]];
						e.points[0] = &grid.points[current_face.points[3]];
					}
					e.points[4] = &grid.points[cell_center_id];
				} else {
					// Split polygon face into tris and quads
					std::vector<OFFace> split_faces = splitPolyFace(current_face,grid);

					//create element from each of the split faces and the cell center
					for (OFFace& new_face : split_faces) {
						if (new_face.n_points == 3)
							grid.elements.emplace_back(TETRA);
						else if (new_face.n_points == 4)
							grid.elements.emplace_back(TETRA);
						Element& e = grid.elements.back();

						if (j < n_owners_per_cell[i]) {
							e.points[0] = &grid.points[new_face.points[0]];
							e.points[1] = &grid.points[new_face.points[1]];
							e.points[2] = &grid.points[new_face.points[2]];
						} else {
							e.points[2] = &grid.points[new_face.points[0]];
							e.points[1] = &grid.points[new_face.points[1]];
							e.points[0] = &grid.points[new_face.points[2]];
						}
						e.points[3] = &grid.points[cell_center_id];
					}
				}
			}
		}
	}
	for (OFBoundary& boundary : boundaries) {
		int name_i = grid.names.size();
		grid.names.emplace_back(2,boundary.name);
		for (int i = boundary.start_face; i < boundary.start_face+boundary.n_faces; ++i) {
			OFFace& face = faces[i];
			if (face.n_points == 3) {
				grid.elements.emplace_back(TRI);
				Element &e = grid.elements.back();
				e.name_i = name_i;
				for (int j = 0; j < 3; ++j)
					e.points[j] = &grid.points[face.points[j]];
			} else if (face.n_points == 4) {
				grid.elements.emplace_back(QUAD);
				Element &e = grid.elements.back();
				e.name_i = name_i;
				for (int j = 0; j < 3; ++j)
					e.points[j] = &grid.points[face.points[j]];
			} else if (face.n_points > 4) {
				std::vector<OFFace> split_faces = splitPolyFace(face,grid);
				for (OFFace split_face : split_faces) {
					if (split_face.n_points == 3) {
						grid.elements.emplace_back(TRI);
						Element &e = grid.elements.back();
						e.name_i = name_i;
						for (int j = 0; j < 3; ++j)
							e.points[j] = &grid.points[face.points[j]];
					} else if (split_face.n_points == 4) {
						grid.elements.emplace_back(TRI);
						Element &e = grid.elements.back();
						e.name_i = name_i;
						for (int j = 0; j < 4; ++j)
							e.points[j] = &grid.points[face.points[j]];
					}
				}
			} else {
				Fatal("Not a valid face for boundary");
			}
		}
	}
	printf("Created Points: %zu\n",grid.points.size());
	printf("Created Elements: %zu\n",grid.elements.size());
}
