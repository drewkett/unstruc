
CC= g++
CXXFLAGS= -O3 -std=gnu++11 -I./include

unstruc_src = $(wildcard src/unstruc/*.cpp)
unstruc_filenames = $(basename $(notdir $(unstruc_src)))
unstruc_obj = $(addsuffix .o,$(addprefix build/unstruc/,$(unstruc_filenames)))

tetmesh_src = $(wildcard src/tetmesh/*.cpp)
tetmesh_filenames = $(basename $(notdir $(tetmesh_src)))
tetmesh_obj = $(addsuffix .o,$(addprefix build/tetmesh/,$(tetmesh_filenames)))

names= convert offset layers volmesh
executables= $(addprefix bin/unstruc-,$(names))

all: $(executables)

build/unstruc/%.o : src/unstruc/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -I./include/unstruc $< -c -o $@

build/lib/libunstruc.a : $(unstruc_obj)
	@mkdir -p $(dir $@)
	ar cr $@ $^

build/tetmesh/%.o : src/tetmesh/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -I./include/tetmesh $< -c -o $@

build/lib/libtetmesh.a : $(tetmesh_obj)
	@mkdir -p $(dir $@)
	ar cr $@ $^

bin/unstruc-volmesh : src/volmesh.cpp build/lib/libunstruc.a build/lib/libtetmesh.a
	@mkdir -p bin
	$(CC) $(CXXFLAGS) $^ -ltet -o $@

bin/unstruc-% : src/%.cpp build/lib/libunstruc.a
	@mkdir -p bin
	$(CC) $(CXXFLAGS) $^ -o $@

clean:
	rm -f build/unstruc build/tetmesh build/lib $(executables)
