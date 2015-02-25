
CC= g++
CXXFLAGS= -O3 -std=gnu++11 -I./include

BUILDDIR= build/make

unstruc_src = $(wildcard src/unstruc/*.cpp)
unstruc_filenames = $(basename $(notdir $(unstruc_src)))
unstruc_obj = $(addsuffix .o,$(addprefix $(BUILDDIR)/unstruc/,$(unstruc_filenames)))

tetmesh_src = $(wildcard src/tetmesh/*.cpp)
tetmesh_filenames = $(basename $(notdir $(tetmesh_src)))
tetmesh_obj = $(addsuffix .o,$(addprefix $(BUILDDIR)/tetmesh/,$(tetmesh_filenames)))

names= convert offset layers volmesh
executables= $(addprefix bin/unstruc-,$(names))

all: $(executables)

$(BUILDDIR)/unstruc/%.o : src/unstruc/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -I./include/unstruc $< -c -o $@

$(BUILDDIR)/lib/libunstruc.a : $(unstruc_obj)
	@mkdir -p $(dir $@)
	ar cr $@ $^

$(BUILDDIR)/tetmesh/%.o : src/tetmesh/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -I./include/tetmesh $< -c -o $@

$(BUILDDIR)/lib/libtetmesh.a : $(tetmesh_obj)
	@mkdir -p $(dir $@)
	ar cr $@ $^

bin/unstruc-volmesh : src/volmesh.cpp $(BUILDDIR)/lib/libunstruc.a $(BUILDDIR)/lib/libtetmesh.a
	@mkdir -p bin
	$(CC) $(CXXFLAGS) $^ -ltet -o $@

bin/unstruc-offset : src/offset.cpp $(BUILDDIR)/lib/libunstruc.a $(BUILDDIR)/lib/libtetmesh.a
	@mkdir -p bin
	$(CC) $(CXXFLAGS) $^ -ltet -o $@

bin/unstruc-% : src/%.cpp $(BUILDDIR)/lib/libunstruc.a
	@mkdir -p bin
	$(CC) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf $(BUILDDIR) $(executables)
