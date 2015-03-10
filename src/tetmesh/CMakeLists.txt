
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/tetmesh)
add_library(tetmesh volume.cpp surface.cpp farfield.cpp)
target_link_libraries(tetmesh unstruc)
