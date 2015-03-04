#ifndef INTERSECTIONS_H_01DF6924_DFF5_4B85_B728_5007A3EB409C
#define INTERSECTIONS_H_01DF6924_DFF5_4B85_B728_5007A3EB409C

#include <vector>

struct Grid;
namespace intersections {
	struct Data {
		std::vector <int> points;
		std::vector <int> elements;
	};

	Data find(const Grid& grid);
}

#endif
