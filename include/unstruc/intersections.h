#ifndef INTERSECTIONS_H_01DF6924_DFF5_4B85_B728_5007A3EB409C
#define INTERSECTIONS_H_01DF6924_DFF5_4B85_B728_5007A3EB409C

#include <vector>

namespace unstruc {
	struct Grid;
	struct Vector;

	typedef std::pair<int,int> PointPair;
	typedef std::vector < PointPair > PointPairList;

	struct Intersections {
		std::vector <int> points;
		std::vector <int> elements;

		static Intersections find(const Grid& grid);
		static Intersections find_with_octree(const Grid& grid);
		static PointPairList find_future(const Grid& surface, Grid offset);
		static double get_scale_factor(double distance);
	};
}

#endif
