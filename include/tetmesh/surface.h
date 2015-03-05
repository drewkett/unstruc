#ifndef SURFACE_H_FBC82A93_B2E4_4E62_95F7_9396D65DCEC9
#define SURFACE_H_FBC82A93_B2E4_4E62_95F7_9396D65DCEC9

#include <vector>

namespace unstruc{
	struct Grid;
	struct Point;
};

namespace tetmesh {
	unstruc::Grid tetrahedralize_surface(unstruc::Grid const& surface, double max_area);
	std::vector <unstruc::Point> orient_surfaces(unstruc::Grid& surface);
}

#endif
