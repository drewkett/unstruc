#ifndef SURFACE_H_FBC82A93_B2E4_4E62_95F7_9396D65DCEC9
#define SURFACE_H_FBC82A93_B2E4_4E62_95F7_9396D65DCEC9

struct Grid;

namespace tetmesh {
	Grid tetrahedralize_surface(Grid const& surface, double max_area);
}

#endif
