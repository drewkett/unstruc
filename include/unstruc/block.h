#ifndef BLOCK_H_17859E7C_9108_4D1B_8805_3FA4F71F11EF
#define BLOCK_H_17859E7C_9108_4D1B_8805_3FA4F71F11EF

#include <vector>

#include "point.h"

namespace unstruc {
	struct Grid;

	struct Block {
		size_t size1, size2, size3;
		std::vector <Point> points;

		Block(size_t s1, size_t s2, size_t s3);
		Point at(size_t i, size_t j, size_t k);
		double * at_ref(size_t i, size_t j, size_t k, size_t l);
		size_t index(size_t i, size_t j, size_t k);
	};

	struct MultiBlock {
		std::vector <Block> blocks;

		Grid to_grid();
	};

}

#endif
