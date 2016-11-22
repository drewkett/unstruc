#include "block.h"

#include "point.h"
#include "element.h"
#include "grid.h"

#include <iostream>
#include <sstream>
#include <vector>

namespace unstruc {

Block::Block(size_t s1, size_t s2, size_t s3) : size1(s1), size2(s2), size3(s3) {
	points = std::vector <Point> (size1*size2*size3);
};

Point Block::at(size_t i, size_t j, size_t k) {
	return points[i*(size2*size3) + j*size3 + k];
};
double * Block::at_ref(size_t i, size_t j, size_t k, size_t l) {
	size_t ii = i*(size2*size3) + j*size3 + k;
	switch (l) {
		case 0:
			return &(points[ii].x);
		case 1:
			return &(points[ii].y);
		case 2:
			return &(points[ii].z);
		default:
			return NULL;
	}
};
size_t Block::index(size_t i, size_t j, size_t k) {
	return i*(size2*size3) + j*size3 + k;
};

Grid MultiBlock::to_grid() {
	Grid grid(3);
	size_t offset = 0;
	std::stringstream ss;
	std::cerr << "Converting to unstructured grid" << std::endl;
	size_t si,sj,sk;
	for (size_t ib = 0; ib < blocks.size(); ib++) {
		Block& blk = blocks[ib];
		si = blk.size1;
		sj = blk.size2;
		sk = blk.size3;
		for (size_t i = 0; i < si; i++)
			for (size_t j = 0; j < sj; j++)
				for (size_t k = 0; k < sk; k++)
					grid.points.push_back(blk.at(i,j,k));
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1;
		grid.names.push_back(Name(3,ss.str()));
		for (size_t i = 0; i < si-1; i++) {
			for (size_t j = 0; j < sj-1; j++) {
				for (size_t k = 0; k < sk-1; k++) {
					Element e = Element(Shape::Hexa);
					e.points[0] = offset+blk.index(i,j,k);
					e.points[1] = offset+blk.index(i+1,j,k);
					e.points[2] = offset+blk.index(i+1,j+1,k);
					e.points[3] = offset+blk.index(i,j+1,k);
					e.points[4] = offset+blk.index(i,j,k+1);
					e.points[5] = offset+blk.index(i+1,j,k+1);
					e.points[6] = offset+blk.index(i+1,j+1,k+1);
					e.points[7] = offset+blk.index(i,j+1,k+1);
					e.name_i = grid.names.size()-1;
					grid.elements.push_back(e);
				}
			}
		}
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceI1";
		grid.names.push_back(Name(2,ss.str()));
		size_t i = 0;
		for (size_t j = 0; j < sj-1; j++) {
			for (size_t k = 0; k < sk-1; k++) {
				Element e = Element(Shape::Quad);
				e.points[0] = offset+blk.index(i,j,k);
				e.points[1] = offset+blk.index(i,j+1,k);
				e.points[2] = offset+blk.index(i,j+1,k+1);
				e.points[3] = offset+blk.index(i,j,k+1);
				e.name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceI2";
		grid.names.push_back(Name(2,ss.str()));
		i = si-1;
		for (size_t j = 0; j < sj-1; j++) {
			for (size_t k = 0; k < sk-1; k++) {
				Element e = Element(Shape::Quad);
				e.points[0] = offset+blk.index(i,j,k);
				e.points[1] = offset+blk.index(i,j+1,k);
				e.points[2] = offset+blk.index(i,j+1,k+1);
				e.points[3] = offset+blk.index(i,j,k+1);
				e.name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceJ1";
		grid.names.push_back(Name(2,ss.str()));
		size_t j = 0;
		for (size_t i = 0; i < si-1; i++) {
			for (size_t k = 0; k < sk-1; k++) {
				Element e = Element(Shape::Quad);
				e.points[0] = offset+blk.index(i,j,k);
				e.points[1] = offset+blk.index(i+1,j,k);
				e.points[2] = offset+blk.index(i+1,j,k+1);
				e.points[3] = offset+blk.index(i,j,k+1);
				e.name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceJ2";
		grid.names.push_back(Name(2,ss.str()));
		j = sj-1;
		for (size_t i = 0; i < si-1; i++) {
			for (size_t k = 0; k < sk-1; k++) {
				Element e = Element(Shape::Quad);
				e.points[0] = offset+blk.index(i,j,k);
				e.points[1] = offset+blk.index(i+1,j,k);
				e.points[2] = offset+blk.index(i+1,j,k+1);
				e.points[3] = offset+blk.index(i,j,k+1);
				e.name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceK1";
		grid.names.push_back(Name(2,ss.str()));
		size_t k = 0;
		for (size_t i = 0; i < si-1; i++) {
			for (size_t j = 0; j < sj-1; j++) {
				Element e = Element(Shape::Quad);
				e.points[0] = offset+blk.index(i,j,k);
				e.points[1] = offset+blk.index(i+1,j,k);
				e.points[2] = offset+blk.index(i+1,j+1,k);
				e.points[3] = offset+blk.index(i,j+1,k);
				e.name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		ss.str("");
		ss.clear();
		ss << "Block" << ib+1 << " FaceK2";
		grid.names.push_back(Name(2,ss.str()));
		k = sk-1;
		for (size_t i = 0; i < si-1; i++) {
			for (size_t j = 0; j < sj-1; j++) {
				Element e = Element(Shape::Quad);
				e.points[0] = offset+blk.index(i,j,k);
				e.points[1] = offset+blk.index(i+1,j,k);
				e.points[2] = offset+blk.index(i+1,j+1,k);
				e.points[3] = offset+blk.index(i,j+1,k);
				e.name_i = grid.names.size()-1;
				grid.elements.push_back(e);
			}
		}
		offset += si*sj*sk;
	}
	std::cerr << grid.points.size() << " Points" << std::endl;
	std::cerr << grid.elements.size() << " Elements" << std::endl;
	return grid;
}

} // namespace unstruc
