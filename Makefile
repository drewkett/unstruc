
CC= g++
CXXFLAGS= -O3 -std=gnu++11 -I./include/grid

grid_src = $(wildcard src/grid/*.cpp)

names= unstruc offset layers volmesh
executables= $(addprefix bin/,$(names))

all: $(executables)

bin/volmesh : src/volmesh/*.cpp $(grid_src)
	$(CC) $(CXXFLAGS) $? -ltet -o $@

bin/% : src/%/*.cpp $(grid_src)
	$(CC) $(CXXFLAGS) $? -o $@

clean:
	rm -f $(executables)
