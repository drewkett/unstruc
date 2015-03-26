
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/unstruc)
add_library(unstruc grid.cpp element.cpp point.cpp error.cpp vtk.cpp stl.cpp plot3d.cpp su2.cpp openfoam.cpp gmsh.cpp block.cpp io.cpp intersections.cpp quality.cpp)
