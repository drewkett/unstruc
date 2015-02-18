This is a collection of utilities related to unstructured meshes. It is written in C++. There are no external dependencies except for [tetgen](http://wias-berlin.de/software/tetgen/), which is needed by volmesh.

##unstruc

Unstructured mesh conversion tool

This tool is a CFD mesh conversion tool. It currently supports reading in [SU2](https://github.com/su2code/SU2), [OpenFoam](http://www.openfoam.com), ASCII STL files, and Plot3D meshes and outputting [SU2](https://github.com/su2code/SU2) and VTK files.

##layers

This is a work in progress tool for generating prismatic boundary layers of a 3D surface mesh by incrementally adding layers where possible.

##offset

This is a work in progress tool for generating prismatic boundary layers of a 3D surface mesh by creating a complete offset surface 

##volmesh

This is a work in progress tool for generating a volume mesh from a farfield surface and a surface object for use with CFD.

## Build Instructions
It can be built using [CMake](http://www.cmake.org)

```
cmake .
make
```
