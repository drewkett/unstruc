
CC= g++
CXXFLAGS= -O3 -std=gnu++11 -I./include

lib_src = $(wildcard src/unstruc/*.cpp)
lib_filenames = $(basename $(notdir $(lib_src)))
lib_obj = $(addsuffix .o,$(addprefix build/unstruc/,$(lib_filenames)))

names= convert offset layers volmesh
executables= $(addprefix bin/unstruc-,$(names))

all: $(executables)

build/unstruc/%.o : src/unstruc/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) $< -c -o $@

lib/libunstruc.a : $(lib_obj)
	ar crf $@ $^

bin/unstruc-volmesh : src/volmesh.cpp lib/libunstruc.a
	$(CC) $(CXXFLAGS) $^ -ltet -o $@

bin/unstruc-% : src/%.cpp lib/libunstruc.a
	$(CC) $(CXXFLAGS) $^ -o $@

clean:
	rm -f build/unstruc/*.o lib/libunstruc.a $(executables)
