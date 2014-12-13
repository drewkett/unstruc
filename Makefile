CC=g++
CFLAGS=-O2

.DEFAULT : unstruc

unstruc :  vtk.cpp plot3d.cpp translation.cpp block.cpp gmsh.cpp su2.cpp grid.cpp error.cpp point.cpp element.cpp unstruc.cpp
	$(CC) $(CFLAGS) $^ -o $@
