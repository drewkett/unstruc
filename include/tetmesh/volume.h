#ifndef VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B
#define VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B

#include <vector>

struct Grid;
struct Point;
struct tetgenio;

namespace tetmesh {
	Grid grid_from_tetgenio(const tetgenio& tg);
	Grid volgrid_from_surface(const Grid& surface,const std::vector<Point>& holes,double min_ratio);
	Grid volgrid_from_surface(const Grid& surface);
	Point find_point_inside_surface(const Grid& surface);
	Point orient_surface(Grid& surface);
}

#endif
