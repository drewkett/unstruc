#ifndef STL_H_A05EAA8C_5EFA_4504_B4BE_34E9DDF0FD9C
#define STL_H_A05EAA8C_5EFA_4504_B4BE_34E9DDF0FD9C

#include <string>

struct Grid;

Grid read_stl_ascii(std::string filename);
Grid read_stl_binary(std::string filename);
Grid read_stl(std::string filename);

#endif
