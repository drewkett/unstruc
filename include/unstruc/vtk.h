#ifndef VTK_H_CB3FDB85_C46B_4D31_9EFB_12CCC08A080E
#define VTK_H_CB3FDB85_C46B_4D31_9EFB_12CCC08A080E

#include <string>
#include <vector>

namespace unstruc {
	struct Grid;
	struct Vector;

	bool vtk_write(const std::string& filename, const Grid &grid);
	Grid vtk_read(const std::string& filename);

	void vtk_write_point_data_header(const std::string& filename, const Grid &grid);
	void vtk_write_cell_data_header(const std::string& filename, const Grid &grid);
	void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <int>& scalars);
	void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <double>& scalars);
	void vtk_write_data(const std::string& filename, const std::string& name, const std::vector <Vector>& vectors);
}

#endif
