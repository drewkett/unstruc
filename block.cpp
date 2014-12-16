#include "block.h"

#include "element.h"
#include "grid.h"

#include <iostream>
#include <sstream>
#include <vector>

Block::Block(int s1, int s2, int s3) : size1(s1), size2(s2), size3(s3) {
	points = std::vector <Point> (size1*size2*size3);
};
Point * Block::at(int i, int j, int k) {
	return &(points[i*(size2*size3) + j*size3 + k]);
};
double * Block::at(int i, int j, int k, int l) {
	switch (l) {
		case 0:
			return &(points[i*(size2*size3) + j*size3 + k].x);
		case 1:
			return &(points[i*(size2*size3) + j*size3 + k].y);
		case 2:
			return &(points[i*(size2*size3) + j*size3 + k].z);
		default:
			return NULL;
	}
};
int Block::index(int i, int j, int k) {
	return i*(size2*size3) + j*size3 + k;
};

int size(MultiBlock &mb) {
	return mb.blocks.size();
}

Grid * to_grid(MultiBlock &mb) {
	Grid * grid = new Grid();
	grid->dim = 3;
	int offset = 0;
	Name name;
	std::stringstream ss;
	Element * e;
	std::cerr << "Converting to unstructured grid" << std::endl;
	int si,sj,sk;
	for (int ib = 0; ib < size(mb); ib++) {
		Block blk = mb.blocks[ib];
		si = blk.size1;
		sj = blk.size2;
		sk = blk.size3;
		for (int i = 0; i < si; i++) {
			for (int j = 0; j < sj; j++) {
				for (int k = 0; k < sk; k++) {
					grid->points.push_back(blk.at(i,j,k));
					grid->ppoints.push_back(&grid->points.back());
				}
			}
		}
		name = Name();
		name.dim = 3;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1;
		name.name = ss.str();
		grid->names.push_back(name);
		for (int i = 0; i < si-1; i++) {
			for (int j = 0; j < sj-1; j++) {
				for (int k = 0; k < sk-1; k++) {
					e = new Element(HEXA);
					e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
					e->points[1] = grid->ppoints[offset+blk.index(i+1,j,k)];
					e->points[2] = grid->ppoints[offset+blk.index(i+1,j+1,k)];
					e->points[3] = grid->ppoints[offset+blk.index(i,j+1,k)];
					e->points[4] = grid->ppoints[offset+blk.index(i,j,k+1)];
					e->points[5] = grid->ppoints[offset+blk.index(i+1,j,k+1)];
					e->points[6] = grid->ppoints[offset+blk.index(i+1,j+1,k+1)];
					e->points[7] = grid->ppoints[offset+blk.index(i,j+1,k+1)];
					e->name_i = grid->names.size()-1;
					grid->elements.push_back(e);
				}
			}
		}
		name = Name();
		name.dim = 2;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceI1";
		name.name = ss.str();
		grid->names.push_back(name);
		int i = 0;
		for (int j = 0; j < sj-1; j++) {
			for (int k = 0; k < sk-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
				e->points[1] = grid->ppoints[offset+blk.index(i,j+1,k)];
				e->points[2] = grid->ppoints[offset+blk.index(i,j+1,k+1)];
				e->points[3] = grid->ppoints[offset+blk.index(i,j,k+1)];
				e->name_i = grid->names.size()-1;
				grid->elements.push_back(e);
			}
		}
		name = Name();
		name.dim = 2;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceI2";
		name.name = ss.str();
		grid->names.push_back(name);
		i = si-1;
		for (int j = 0; j < sj-1; j++) {
			for (int k = 0; k < sk-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
				e->points[1] = grid->ppoints[offset+blk.index(i,j+1,k)];
				e->points[2] = grid->ppoints[offset+blk.index(i,j+1,k+1)];
				e->points[3] = grid->ppoints[offset+blk.index(i,j,k+1)];
				e->name_i = grid->names.size()-1;
				grid->elements.push_back(e);
			}
		}
		name = Name();
		name.dim = 2;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceJ1";
		name.name = ss.str();
		grid->names.push_back(name);
		int j = 0;
		for (int i = 0; i < si-1; i++) {
			for (int k = 0; k < sk-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
				e->points[1] = grid->ppoints[offset+blk.index(i+1,j,k)];
				e->points[2] = grid->ppoints[offset+blk.index(i+1,j,k+1)];
				e->points[3] = grid->ppoints[offset+blk.index(i,j,k+1)];
				e->name_i = grid->names.size()-1;
				grid->elements.push_back(e);
			}
		}
		name = Name();
		name.dim = 2;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceJ2";
		name.name = ss.str();
		grid->names.push_back(name);
		j = sj-1;
		for (int i = 0; i < si-1; i++) {
			for (int k = 0; k < sk-1; k++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
				e->points[1] = grid->ppoints[offset+blk.index(i+1,j,k)];
				e->points[2] = grid->ppoints[offset+blk.index(i+1,j,k+1)];
				e->points[3] = grid->ppoints[offset+blk.index(i,j,k+1)];
				e->name_i = grid->names.size()-1;
				grid->elements.push_back(e);
			}
		}
		name = Name();
		name.dim = 2;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceK1";
		name.name = ss.str();
		grid->names.push_back(name);
		int k = 0;
		for (int i = 0; i < si-1; i++) {
			for (int j = 0; j < sj-1; j++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
				e->points[1] = grid->ppoints[offset+blk.index(i+1,j,k)];
				e->points[2] = grid->ppoints[offset+blk.index(i+1,j+1,k)];
				e->points[3] = grid->ppoints[offset+blk.index(i,j+1,k)];
				e->name_i = grid->names.size()-1;
				grid->elements.push_back(e);
			}
		}
		name = Name();
		name.dim = 2;
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceK2";
		name.name = ss.str();
		grid->names.push_back(name);
		k = sk-1;
		for (int i = 0; i < si-1; i++) {
			for (int j = 0; j < sj-1; j++) {
				Element * e = new Element(QUAD);
				e->points[0] = grid->ppoints[offset+blk.index(i,j,k)];
				e->points[1] = grid->ppoints[offset+blk.index(i+1,j,k)];
				e->points[2] = grid->ppoints[offset+blk.index(i+1,j+1,k)];
				e->points[3] = grid->ppoints[offset+blk.index(i,j+1,k)];
				e->name_i = grid->names.size()-1;
				grid->elements.push_back(e);
			}
		}
		offset += si*sj*sk;
	}
	std::cerr << grid->points.size() << " Points" << std::endl;
	std::cerr << grid->elements.size() << " Elements" << std::endl;
	return grid;
}
