#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <algorithm>

#include "error.h"
#include "point.h"
#include "element.h"
using namespace std;


#define TOL 3.e-8

struct Name {
	int dim;
	int i;
	char name[20];
};

struct Grid {
	vector <Point *> points;
	vector <Point **> ppoints;
	vector <Element *> elements;
	vector <Name *> names;
	int n_points;
	int n_elems;
	int n_names;
};

bool set_i(Grid * grid) {
	int i_p = 0;
	for (int i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		(*grid->ppoints[i])->i = i_p;
		i_p++;
	}
	grid->n_points = i_p;
	bool* name_mask = new bool[grid->names.size()];
	for (int i = 0; i < grid->names.size(); i++) {
		name_mask[i] = false;
	}
	int i_e = 0;
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!grid->elements[i]) continue;
		name_mask[grid->elements[i]->name_i] = true;
		if (!is3D(grid->elements[i])) continue;
		grid->elements[i]->i = i_e;
		i_e++;
	}
	grid->n_elems = i_e;
	int i_n = 0;
	for (int i = 0; i < grid->names.size(); i++) {
		if (name_mask[i] && grid->names[i]->dim == 2) {
			grid->names[i]->i = i_n;
			i_n++;
		} else {
			delete grid->names[i];
			grid->names[i] = NULL;
		}
	}
	grid->n_names = i_n;
	return true;
}

bool toSU2(Grid * grid) {
	std::cout.precision(15);
	Point * p;
	Element * e;
	Name * name;
	set_i(grid);
	sort(grid->elements.begin(),grid->elements.end(),compareElementByName);
	cout << "NDIME= " << 3 << endl;
	cout << endl;
	cout << "NELEM= " << grid->n_elems << endl;
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!is3D(grid->elements[i])) continue;
		e = grid->elements[i];
		cout << e->type;
		for (int j = 0; j < e->len; j++) {
			cout << " " << (**e->points[j]).i;
		}
		cout << " " << i;
		cout << endl;
	}
	cout << endl;
	cout << "NPOIN= " << grid->n_points << endl;
	for (int i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		p = *grid->ppoints[i];
		cout << p->x << " " << p->y << " " << p->z << " " << p->i << endl;
	}
	cout << endl;
	int n_names = 0;
	for (int i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		if (grid->names[i]->dim != 2) continue;
		n_names++;
	}
	int* name_count = new int[grid->names.size()];
	for (int i = 0; i < grid->names.size(); i++) {
		name_count[i] = 0;
	}
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!is2D(grid->elements[i])) continue;
		e = grid->elements[i];
		name_count[e->name_i]++;
	}
	cout << "NMARK= " << n_names << endl;
	cerr << "MARKERS:" << endl;
	for (int i = 0; i < grid->names.size(); i++) {
		if (!grid->names[i]) continue;
		name = grid->names[i];
		if (name->dim != 2) continue;
		cerr << i << " : " << name->name << endl;
		cout << "MARKER_TAG= " << name->name << endl;
		cout << "MARKER_ELEMS= " << name_count[i] << endl;
		for (int j = 0; j < grid->elements.size(); j++) {
			if (!is2D(grid->elements[j])) continue;
			e = grid->elements[j];
			if (e->name_i != i) continue;
			cout << e->type;
			for (int k = 0; k < e->len; k++) {
				cout << " " << (**e->points[k]).i;
			}
			cout << endl;
		}
	}
	return true;
}

bool toGMSH(Grid * grid) {
	Point * p;
	Element * e;
	Name * name;
	set_i(grid);
	cout << "$MeshFormat" << endl;
	cout << "2.2 0 " << sizeof(double) << endl;
	cout << "$EndMeshFormat" << endl;
	cout << "$Nodes" << endl;
	cout << grid->n_points << endl;
	for (int i = 0; i < grid->ppoints.size(); i++) {
		if (!grid->ppoints[i]) continue;
		p = *grid->ppoints[i];
		cout << p->i+1 << " " << p->x << " " << p->y << " " << p->z << endl;
	}
	cout << "$EndNodes" << endl;
	cout << "$Elements" << endl;
	cout << grid->n_elems << endl;
	for (int i = 0; i < grid->elements.size(); i++) {
		if (!grid->elements[i]) continue;
		e = grid->elements[i];
		cout << e->i+1;
		switch (e->type) {
			case QUAD:
				cout << " 3";
				break;
			case HEXA:
				cout << " 5";
				break;
			default:
				NotImplemented("ElType for GMSH");
		}
		cout << " " << 2 << " " << e->name_i+1 << " " << e->name_i+1;
		for (int j = 0; j < e->len; j++) {
			cout << " " << (**e->points[j]).i+1;
		}
		cout << endl;
	}
	cout << "$EndElements" << endl;
	cout << "$PhysicalNames" << endl;
	cout << grid->names.size() << endl;
	for (int i = 0; i < grid->names.size(); i++) {
		name = grid->names[i];
		cout << name->dim << " " << i+1 << " \"" << name->name << "\"" << endl;
	}
	cout << "$EndPhysicalNames" << endl;
	return true;
}

struct Block {
	int size1, size2, size3;
	Point * points;
	Block(int s1, int s2, int s3) : size1(s1), size2(s2), size3(s3) {
		points = new Point[size1*size2*size3];
	};
	Point * at(int i, int j, int k) {
		return &(points[i*(size2*size3) + j*size3 + k]);
	};
	double * at(int i, int j, int k, int l) {
		switch (l) {
			case 0:
				return &(points[i*(size2*size3) + j*size3 + k].x);
			case 1:
				return &(points[i*(size2*size3) + j*size3 + k].y);
			case 2:
				return &(points[i*(size2*size3) + j*size3 + k].z);
			default:
				cerr << "No" << endl;
				exit(1);
		}
	};
	int index(int i, int j, int k) {
		return i*(size2*size3) + j*size3 + k;
	};
};

struct TranslationTable {
	vector <Name *> names;
	vector <int> index;
	TranslationTable(int n) : names(0), index(n,-1) {};
};

TranslationTable * ReadTranslationFile(char * filename, int n_blocks) {
	ifstream f;
	Name * name;
	TranslationTable * tt = new TranslationTable(7*n_blocks);
	string line, s;
	cerr << "Reading Translation File '" << filename << "'" << endl;
	f.open(filename,ios::in);
	if (f.is_open()) {
		while (getline(f,line)) {
			name = new Name();
			name->dim = 2;
			istringstream iss(line);
			iss >> s;
			strncpy(name->name, s.c_str(), 20);
			tt->names.push_back(name);
			cerr << s;
			while (! iss.eof()) {
				iss >> s;
				//if (iss.eof()) break;
				cerr << " " << s;
				tt->index[atoi(s.c_str())] = tt->names.size()-1;
			}
			cerr << endl;
		}
		f.close();
	} else {
		Fatal("Could not open file");
	}
	return tt;
}

int main (int argc, char* argv[])
{
	if (argc < 2) {
		Fatal("One argument required");
	}
	int i = 1;
	char * blockfile = NULL;
	char * translationfile = NULL;
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (string(argv[i]) == "-t") {
				i++;
				if (i == argc) Fatal("Must filename option to -t");
				translationfile  = argv[i];
			}
		} else {
			if (blockfile) Fatal("blockname defined twice");
			blockfile = argv[i];
		}
		i++;
	}
	ifstream f;
	cerr << "Opening Block" << endl;
	f.open(blockfile,ios::in|ios::binary);
	int n_blocks,offset=0;
	f.read((char *) &n_blocks,4);
	int dim[n_blocks][3];
	cerr << n_blocks << " blocks" << endl;
	cerr << "Dimensions: " << endl;
	for (int i = 0; i < n_blocks; i++) {
		f.read((char *) &dim[i],12);
		cerr << dim[i][0] << " " << dim[i][1] << " " << dim[i][2] << endl;
	}
	Grid grid;
	Block * blks[n_blocks];
	Name * name;
	Point * p;
	Element * e;
	Block * blk;
	int n_points = 0;
	for (int ib = 0; ib < n_blocks; ib++) {
		n_points += dim[ib][0]*dim[ib][1]*dim[ib][2];
		if (dim[ib][0] > 10000) {
			cerr << "Something is wrong with idir in block " << ib << endl;
			exit(1);
		}
		if (dim[ib][1] > 10000) {
			cerr << "Something is wrong with jdir in block " << ib << endl;
			exit(1);
		}
		if (dim[ib][2] > 10000) {
			cerr << "Something is wrong with kdir in block " << ib << endl;
			exit(1);
		}
	}
	grid.points.resize(n_points);
	grid.ppoints.resize(n_points);
	for (int ib = 0; ib < n_blocks; ib++) {
		//blk = new Block(dim[ib][0],dim[ib][1],dim[ib][2]); 
		blk = new Block(dim[ib][0],dim[ib][1],dim[ib][2]); 
		for (int l = 0; l < 3; l++) {
			for (int k = 0; k < dim[ib][2]; k++) {
				for (int j = 0; j < dim[ib][1]; j++) {
					for (int i = 0; i < dim[ib][0]; i++) {
						f.read((char *) blk->at(i,j,k,l),sizeof(double));
					}
				}
			}
		}
		for (int i = 0; i < dim[ib][0]; i++) {
			for (int j = 0; j < dim[ib][1]; j++) {
				for (int k = 0; k < dim[ib][2]; k++) {
					grid.points[offset+blk->index(i,j,k)] = blk->at(i,j,k);
				}
			}
		}
		for (int i = 0; i < dim[ib][0]*dim[ib][1]*dim[ib][2]; i++) {
			grid.ppoints[offset+i] = &grid.points[offset+i];
		}
		name = new Name();
		name->dim = 3;
		sprintf(name->name,"Block%d",ib+1);
		grid.names.push_back(name);
		for (int i = 0; i < dim[ib][0]-1; i++) {
			for (int j = 0; j < dim[ib][1]-1; j++) {
				for (int k = 0; k < dim[ib][2]-1; k++) {
					Element * e = new Element(HEXA);
					e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
					e->points[1] = grid.ppoints[offset+blk->index(i+1,j,k)];
					e->points[2] = grid.ppoints[offset+blk->index(i+1,j+1,k)];
					e->points[3] = grid.ppoints[offset+blk->index(i,j+1,k)];
					e->points[4] = grid.ppoints[offset+blk->index(i,j,k+1)];
					e->points[5] = grid.ppoints[offset+blk->index(i+1,j,k+1)];
					e->points[6] = grid.ppoints[offset+blk->index(i+1,j+1,k+1)];
					e->points[7] = grid.ppoints[offset+blk->index(i,j+1,k+1)];
					e->name_i = grid.names.size()-1;
					grid.elements.push_back(e);
				}
			}
		}
		name = new Name();
		name->dim = 2;
		sprintf(name->name,"Block%d FaceI1",ib+1);
		grid.names.push_back(name);
		int i = 0;
		for (int j = 0; j < dim[ib][1]-1; j++) {
			for (int k = 0; k < dim[ib][2]-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
				e->points[1] = grid.ppoints[offset+blk->index(i,j+1,k)];
				e->points[2] = grid.ppoints[offset+blk->index(i,j+1,k+1)];
				e->points[3] = grid.ppoints[offset+blk->index(i,j,k+1)];
				e->name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		name = new Name();
		name->dim = 2;
		sprintf(name->name,"Block%d FaceI2",ib+1);
		grid.names.push_back(name);
		i = dim[ib][0]-1;
		for (int j = 0; j < dim[ib][1]-1; j++) {
			for (int k = 0; k < dim[ib][2]-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
				e->points[1] = grid.ppoints[offset+blk->index(i,j+1,k)];
				e->points[2] = grid.ppoints[offset+blk->index(i,j+1,k+1)];
				e->points[3] = grid.ppoints[offset+blk->index(i,j,k+1)];
				e->name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		name = new Name();
		name->dim = 2;
		sprintf(name->name,"Block%d FaceJ1",ib+1);
		grid.names.push_back(name);
		int j = 0;
		for (int i = 0; i < dim[ib][0]-1; i++) {
			for (int k = 0; k < dim[ib][2]-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
				e->points[1] = grid.ppoints[offset+blk->index(i+1,j,k)];
				e->points[2] = grid.ppoints[offset+blk->index(i+1,j,k+1)];
				e->points[3] = grid.ppoints[offset+blk->index(i,j,k+1)];
				e->name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		name = new Name();
		name->dim = 2;
		sprintf(name->name,"Block%d FaceJ2",ib+1);
		grid.names.push_back(name);
		j = dim[ib][1]-1;
		for (int i = 0; i < dim[ib][0]-1; i++) {
			for (int k = 0; k < dim[ib][2]-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
				e->points[1] = grid.ppoints[offset+blk->index(i+1,j,k)];
				e->points[2] = grid.ppoints[offset+blk->index(i+1,j,k+1)];
				e->points[3] = grid.ppoints[offset+blk->index(i,j,k+1)];
				e->name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		name = new Name();
		name->dim = 2;
		sprintf(name->name,"Block%d FaceK1",ib+1);
		grid.names.push_back(name);
		int k = 0;
		for (int i = 0; i < dim[ib][0]-1; i++) {
			for (int j = 0; j < dim[ib][1]-1; j++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
				e->points[1] = grid.ppoints[offset+blk->index(i+1,j,k)];
				e->points[2] = grid.ppoints[offset+blk->index(i+1,j+1,k)];
				e->points[3] = grid.ppoints[offset+blk->index(i,j+1,k)];
				e->name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		name = new Name();
		name->dim = 2;
		sprintf(name->name,"Block%d FaceK2",ib+1);
		grid.names.push_back(name);
		k = dim[ib][2]-1;
		for (int i = 0; i < dim[ib][0]-1; i++) {
			for (int j = 0; j < dim[ib][1]-1; j++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid.ppoints[offset+blk->index(i,j,k)];
				e->points[1] = grid.ppoints[offset+blk->index(i+1,j,k)];
				e->points[2] = grid.ppoints[offset+blk->index(i+1,j+1,k)];
				e->points[3] = grid.ppoints[offset+blk->index(i,j+1,k)];
				e->name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		offset += dim[ib][0]*dim[ib][1]*dim[ib][2];
	}
	for (int i=0; i < grid.points.size(); i++) {
		set_s(grid.points[i]);
	}
	cerr << grid.points.size() << " Points" << endl;
	cerr << grid.elements.size() << " Elements" << endl;
	sort(grid.ppoints.begin(),grid.ppoints.end(),comparePPoint);
	int n = 0;
	for (int i = 0; i < grid.ppoints.size()-1; i++) {
		if (!grid.ppoints[i]) continue;
		for (int j = i+1; j < grid.ppoints.size(); j++) {
			if (!grid.ppoints[j]) continue;
			if (!close(*grid.ppoints[i],*grid.ppoints[j],TOL)) break;
			if (same(*grid.ppoints[i],*grid.ppoints[j],TOL)) {
				*grid.ppoints[j] = *grid.ppoints[i];
				grid.ppoints[j] = NULL;
				n++;
			}
		}
	}
	cerr << n << " Points Merged" << endl;
	for (int i=0; i < grid.elements.size(); i++) {
		set_s(grid.elements[i]);
	}
	sort(grid.elements.begin(),grid.elements.end(),compareElement);
	n = 0;
	for (int i = 0; i < grid.elements.size() - 1; i++) {
		if (!grid.elements[i]) continue;
		bool kill = false;
		for (int j = i+1; j < grid.elements.size(); j++) {
			if (!grid.elements[j]) continue;
		    if (!close(grid.elements[i],grid.elements[j])) break;
			if (same(grid.elements[i],grid.elements[j])) {
				delete grid.elements[j];
				grid.elements[j] = NULL;
				kill = true;
				n++;
			}
		}
		if (kill) {
			kill = false;
			n++;
			delete grid.elements[i];
			grid.elements[i] = NULL;
		}
	}
	cerr << n << " Faces Deleted" << endl;
	bool collapsed;
	n = 0;
	int n2 = 0;
	Element * e_new;
	for (int i = 0; i < grid.elements.size(); i++) {
		e = grid.elements[i];
		if (!e) continue;
		if (canCollapse(e)) {
			e_new = collapse(e);
			n++;
			if (e_new) {
				grid.elements.push_back(e_new);
				n2++;
			}
		}
	}
	cerr << n << " Elements Collapsed" << endl;
	cerr << n2 << " Elements Created on Collapse" << endl;
	if (translationfile) {
		TranslationTable * transt = ReadTranslationFile(translationfile,n_blocks);
		int offset = grid.names.size();
		for (int i=0; i < transt->names.size(); i++) {
			grid.names.push_back(transt->names[i]);
		}
		for (int i=0; i < transt->index.size(); i++) {
			if (transt->index[i] == -1) {
				transt->index[i] = i;
			} else {
				transt->index[i] += offset;
			}
		}
		for (int i = 0; i < grid.elements.size(); i++) {
			e = grid.elements[i];
			if (!e) continue;
			if (e->name_i != -1) e->name_i = transt->index[e->name_i];
		}
	}
	toSU2(&grid);
	return 0;
}
