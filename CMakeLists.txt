# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

add_compile_options(--std=gnu++11 -Wall -O3 -g)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/grid)
ADD_SUBDIRECTORY(src/grid)

add_executable(layers src/layers/layers.cpp)
target_link_libraries(layers grid)

add_executable(unstruc src/unstruc/unstruc.cpp)
target_link_libraries(unstruc grid)

add_executable(offset src/offset/offset.cpp)
target_link_libraries(offset grid)

add_executable(volmesh src/volmesh/volmesh.cpp)
target_link_libraries(volmesh grid tet)
