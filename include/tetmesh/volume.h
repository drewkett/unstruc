#ifndef VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B
#define VOLUME_H_F7F23BA5_0307_481E_8A35_32D78739B72B

#include <vector>
#include "tetgen.h"

namespace unstruc{
	struct Grid;
	struct Point;
};

namespace tetmesh {
	unstruc::Grid grid_from_tetgenio(const tetgenio& tg);
	unstruc::Grid volgrid_from_surface(const unstruc::Grid& surface,const std::vector<unstruc::Point>& holes,double min_ratio);
	unstruc::Grid volgrid_from_surface(const unstruc::Grid& surface);
	unstruc::Point find_point_inside_surface(const unstruc::Grid& surface);
	unstruc::Point orient_surface(unstruc::Grid& surface);
}

#endif
