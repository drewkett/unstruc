
#include "openfoam.h"
#include "grid.h"
#include "element.h"
#include "point.h"
#include "error.h"

#include <string.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

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

struct OFFaces {
	std::vector<int> index;
	std::vector<int> points;
};

struct OFInfo {
	int n_points, n_cells, n_faces, n_internal_faces;
};

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

	//while (std::getline(f,line)) {
	//	if (line.size() == 0) continue;
	//	if (line[0] == '/' && line[1] == '/') continue;
	//	header.n = atoi(line.c_str());
	//	break;
	//}

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

OFInfo readInfoFromOwners(path dirpath) {
	path filepath = dirpath/"owner";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath.string());
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

std::vector<int> readOwners(path dirpath) {
	path filepath = dirpath/"owner";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath.string());
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<int>(f,header);
}

std::vector<int> readNeighbours(path dirpath) {
	path filepath = dirpath/"neighbour";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath.string());
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<int>(f,header);
}

std::vector<OFPoint> readPoints(path dirpath) {
	path filepath = dirpath/"points";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath.string());
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	return readBinary<OFPoint>(f,header);
}

OFFaces readFaces(path dirpath) {
	path filepath = dirpath/"faces";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath.string());
	FoamHeader header = readFoamHeader(f);

	if (header.format == "ascii") Fatal("ascii not supported");

	OFFaces faces;
	faces.index = readBinary<int>(f,header);
	faces.points = readBinary<int>(f,header);

	return faces;
}

std::vector<OFBoundary> readBoundaries(path dirpath) {
	path filepath = dirpath/"boundary";

	std::ifstream f;
	f.open(filepath.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open "+filepath.string());
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

void readOpenFoam(Grid& grid, std::string &filename) {
	path filepath(filename);
	path polymesh = filepath.parent_path()/"constant"/"polyMesh";

	if (!exists(polymesh)) Fatal("constant/polyMesh directory doesn't exist");
	
	OFInfo info = readInfoFromOwners(polymesh);
	std::vector<OFPoint> points = readPoints(polymesh);
	OFFaces faces = readFaces(polymesh);
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

	if (info.n_faces != faces.index.size() - 1 ) Fatal("Invalid FoamFile: number of faces do not match");
	grid.elements.resize(info.n_cells);
	//std::vector<int> n_faces_per_cell (info.n_cells,0);
	//for (int i = 0; i < owners.size(); ++i) {
	//	n_faces_per_cell[owners[i]]++;
	//}
	//for (int i = 0; i < neighbours.size(); ++i) {
	//	n_faces_per_cell[neighbours[i]]++;
	//}
	std::vector< int > n_points_per_face (info.n_faces);
	for (int i = 0; i < info.n_faces; ++i) {
		n_points_per_face[i] = faces.index[i+1] - faces.index[i];
	}

	std::vector< int > n_faces_per_cell (info.n_cells);
	std::vector< std::vector<int> > faces_per_cell (info.n_cells);
	for (int i = 0; i < owners.size(); ++i) {
		n_faces_per_cell[owners[i]]++;
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
		for (int j = 0; j < n_faces_per_cell[i]; ++j) {
			int current_face = cell_faces[j];
			switch (n_points_per_face[current_face]) {
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
		}
	}
	//for (int i = 0; i < info.n_cells; ++i) {
	//	faces_per_cell[i] = new int[n_faces_per_cell[i]];
	//	//printf("%d ",n_faces_per_cell[i]);
	//}
}
