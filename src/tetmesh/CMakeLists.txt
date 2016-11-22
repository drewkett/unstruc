
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/tetmesh)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/tetgen1.5.0)
add_library(tetmesh volume.cpp surface.cpp farfield.cpp)
target_link_libraries(tetmesh tet unstruc)
