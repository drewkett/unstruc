This is a collection of utilities related to unstructured meshes. It is written in C++. There are no external dependencies except for [tetgen](http://wias-berlin.de/software/tetgen/), which is needed by volmesh.

##unstruc-convert

Unstructured mesh conversion tool

This tool is a CFD mesh conversion tool. It currently supports reading in [SU2](https://github.com/su2code/SU2), [OpenFoam](http://www.openfoam.com), STL files, and Plot3D meshes and outputting [SU2](https://github.com/su2code/SU2) and VTK files.

##unstruc-layers

This is a work in progress tool for generating prismatic boundary layers of a 3D surface mesh by incrementally adding layers where possible.

##unstruc-offset

This is a work in progress tool for generating prismatic boundary layers of a 3D surface mesh by creating a complete offset surface 

##unstruc-volmesh

This is a work in progress tool for generating a volume mesh with a farfield mesh using just a surface object for use with CFD.

## Build Instructions
This project can be built using make or cmake. The Makefile is basic and recompiles the executables using all of the files every time. It can also be build using [CMake](http://www.cmake.org), which can be run using `./build.sh`.
