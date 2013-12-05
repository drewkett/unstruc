#include <vector>

struct Point;
struct Grid;

struct Block {
	int size1, size2, size3;
	Point * points;
	Block(int s1, int s2, int s3);
	Point * at(int i, int j, int k);
	double * at(int i, int j, int k, int l);
	int index(int i, int j, int k);
};

struct MultiBlock {
	std::vector <Block *> blocks;
};

int size(MultiBlock *);

Grid * toGrid(MultiBlock *);
