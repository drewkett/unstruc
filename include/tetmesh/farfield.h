#ifndef FARFIELD_H_52D70D7F_E821_4F47_AA26_34B6654EA512
#define FARFIELD_H_52D70D7F_E821_4F47_AA26_34B6654EA512

namespace unstruc {
	struct Grid;
}

namespace tetmesh {
	unstruc::Grid create_farfield_box(unstruc::Grid const& surface);
}

#endif
