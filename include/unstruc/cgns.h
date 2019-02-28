#ifndef CGNS_H_CB3FDB85_C46B_4D31_9EFB_12CCC08A080E
#define CGNS_H_CB3FDB85_C46B_4D31_9EFB_12CCC08A080E

#include <string>
#include <vector>

namespace unstruc {
	struct Grid;
	struct Vector;

	bool cgns_write(const std::string& filename, const Grid &grid);
}

#endif
