unstruc
=======

Unstructured mesh conversion tool

This tool is a CFD mesh conversion tool. It currently supports reading in [SU2](https://github.com/su2code/SU2) and Plot3D meshes and outputting [SU2](https://github.com/su2code/SU2) and VTK files.

## Build Instructions
It can be built using [Ninja](https://github.com/martine/ninja) using the included build.ninja file. Or by running either
```
g++ *.cpp -O2 -o unstruc
```
or
```
clang++ *.cpp -O2 -o unstruc
```
