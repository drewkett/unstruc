# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

add_compile_options(--std=gnu++11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/grid/include)
ADD_SUBDIRECTORY(grid/src)

add_executable(layers layers/layers.cpp)
target_link_libraries(layers grid)

add_executable(unstruc unstruc/unstruc.cpp)
target_link_libraries(unstruc grid)

add_executable(offset offset/offset.cpp)
target_link_libraries(offset grid)

add_executable(volmesh volmesh/volmesh.cpp)
target_link_libraries(volmesh grid tet)
