#ifndef BLOCK_H_17859E7C_9108_4D1B_8805_3FA4F71F11EF
#define BLOCK_H_17859E7C_9108_4D1B_8805_3FA4F71F11EF

#include <vector>

#include "point.h"

struct Grid;

struct Block {
	int size1, size2, size3;
	std::vector <Point> points;

	Block(int s1, int s2, int s3);
	Point at(int i, int j, int k);
	double * at_ref(int i, int j, int k, int l);
	int index(int i, int j, int k);
};

struct MultiBlock {
	std::vector <Block> blocks;

	Grid to_grid();
};

#endif
