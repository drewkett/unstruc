# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

add_compile_options(--std=gnu++11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(src)
