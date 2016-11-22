#ifndef INTERSECTIONS_H_01DF6924_DFF5_4B85_B728_5007A3EB409C
#define INTERSECTIONS_H_01DF6924_DFF5_4B85_B728_5007A3EB409C

#include <vector>

namespace unstruc {
	struct Grid;
	struct Vector;

	typedef std::pair<size_t,size_t> PointPair;
	typedef std::vector < PointPair > PointPairList;

	struct Intersections {
		std::vector <size_t> points;
		std::vector <size_t> elements;

		static Intersections find(const Grid& grid);
		static Intersections find_with_octree(const Grid& grid);
		static PointPairList find_future(const Grid& surface, Grid offset);
		static double get_scale_factor(double distance);
	};
}

#endif
