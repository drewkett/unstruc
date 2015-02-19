# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

add_compile_options(--std=gnu++11 -Wall -O3 -g)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
ADD_SUBDIRECTORY(src/unstruc)

add_executable(unstruc-layers src/layers.cpp)
target_link_libraries(unstruc-layers unstruc)

add_executable(unstruc-convert src/convert.cpp)
target_link_libraries(unstruc-convert unstruc)

add_executable(unstruc-offset src/offset.cpp)
target_link_libraries(unstruc-offset unstruc)

add_executable(unstruc-volmesh src/volmesh.cpp)
target_link_libraries(unstruc-volmesh unstruc tet)
