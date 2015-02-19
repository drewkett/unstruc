
CC= g++
CXXFLAGS= -O3 -g -std=gnu++11

grid_src = $(wildcard grid/src/*.cpp)
grid_include_dir = ./grid/include

names= unstruc offset layers volmesh
executables= $(addprefix bin/,$(names))

all: $(executables)

bin/volmesh : volmesh/*.cpp $(grid_src)
	$(CC) $(CXXFLAGS) -I$(grid_include_dir) $? -ltet -o $@

bin/% : %/*.cpp $(grid_src)
	$(CC) $(CXXFLAGS) -I$(grid_include_dir) $? -o $@

clean:
	rm -f $(executables)
