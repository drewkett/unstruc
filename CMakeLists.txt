# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

add_compile_options(--std=gnu++11)

add_library(grid grid.cpp element.cpp point.cpp error.cpp vtk.cpp stl.cpp plot3d.cpp su2.cpp openfoam.cpp gmsh.cpp block.cpp io.cpp)

add_executable(layers layers.cpp)
target_link_libraries(layers grid)

add_executable(unstruc unstruc.cpp translation.cpp)
target_link_libraries(unstruc grid)

add_executable(offset offset.cpp)
target_link_libraries(offset grid)

#add_subdirectory(tetgen)
add_executable(volmesh volmesh.cpp)
target_link_libraries(volmesh grid tet)
