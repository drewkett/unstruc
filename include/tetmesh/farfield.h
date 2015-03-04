#ifndef FARFIELD_H_52D70D7F_E821_4F47_AA26_34B6654EA512
#define FARFIELD_H_52D70D7F_E821_4F47_AA26_34B6654EA512

struct Grid;

namespace tetmesh {
	Grid create_farfield_box(Grid const& surface);
}

#endif
