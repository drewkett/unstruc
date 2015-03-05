#ifndef STL_H_A05EAA8C_5EFA_4504_B4BE_34E9DDF0FD9C
#define STL_H_A05EAA8C_5EFA_4504_B4BE_34E9DDF0FD9C

#include <string>

namespace unstruc {
	struct Grid;

	Grid stl_read_ascii(const std::string& filename);
	Grid stl_read_binary(const std::string& filename);
	Grid stl_read(const std::string& filename);
	void stl_write_ascii(const std::string& filename, const Grid& grid);
	void stl_write_binary(const std::string& filename, const Grid& grid);
}

#endif
