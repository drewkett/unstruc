
CC= g++
CXXFLAGS= -O3 -std=gnu++11 -I./include

lib_src = $(wildcard src/unstruc/*.cpp)

names= convert offset layers volmesh
executables= $(addprefix bin/unstruc-,$(names))

all: $(executables)

bin/unstruc-volmesh : src/volmesh.cpp $(lib_src)
	$(CC) $(CXXFLAGS) $? -ltet -o $@

bin/unstruc-% : src/%.cpp $(lib_src)
	$(CC) $(CXXFLAGS) $? -o $@

clean:
	rm -f $(executables)
