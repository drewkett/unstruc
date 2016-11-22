# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

# Add an executable to the project using the specified source files.
add_executable(tetgen tetgen.cxx predicates.cxx)

#Add a library to the project using the specified source files. 
# In Linux/Unix, it will creates the libtet.a
add_library(tet STATIC tetgen.cxx predicates.cxx)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#Set properties on a target. 
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(tet PROPERTIES "COMPILE_DEFINITIONS" TETLIBRARY)
