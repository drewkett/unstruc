#ifndef STL_H_A05EAA8C_5EFA_4504_B4BE_34E9DDF0FD9C
#define STL_H_A05EAA8C_5EFA_4504_B4BE_34E9DDF0FD9C

#include <string>

struct Grid;

namespace STL {

Grid read_ascii(const std::string& filename);
Grid read_binary(const std::string& filename);
Grid read(const std::string& filename);
void write_ascii(const std::string& filename, const Grid& grid);
void write_binary(const std::string& filename, const Grid& grid);

}

#endif
