This is a collection of utilities related to unstructured meshes. It is written in C++. The only dependency is [tetgen](http://wias-berlin.de/software/tetgen/), which is used by unstruc-offset.

##unstruc-convert

Unstructured mesh conversion tool

This tool is a CFD mesh conversion tool. It currently supports reading in [SU2](https://github.com/su2code/SU2), [OpenFoam](http://www.openfoam.com), STL files, and Plot3D meshes and outputting [SU2](https://github.com/su2code/SU2) and VTK files.

##unstruc-layers

This is a work in progress tool for generating prismatic boundary layers of a 3D surface mesh by incrementally adding layers where possible.

##unstruc-offset

This is a work in progress tool for generating prismatic boundary layers of a 3D surface mesh by creating a complete offset surface 

## Build Instructions
This project can be built using make or [CMake](http://www.cmake.org). The Makefile is more basic and mainly is useful for building the project one time. Run `make` in the top level directory to build the executables, which are placed in bin.

If you are changing any files, building using cmake works better because it properly handles dependencies particularly if header files change. Run `./cmake_build.sh` in the top level directory to build the executables using cmake.
