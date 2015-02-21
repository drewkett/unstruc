# Set  the minimum  required version  of cmake  for a  project.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_COMPILE_OPTIONS(--std=gnu++11)
PROJECT(unstruc)

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	set(CMAKE_EXECUTABLE_SUFFIX -dbg)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

ADD_SUBDIRECTORY(src/unstruc)
ADD_SUBDIRECTORY(src/tetmesh)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

add_executable(unstruc-layers src/layers.cpp)
target_link_libraries(unstruc-layers unstruc)

add_executable(unstruc-convert src/convert.cpp)
target_link_libraries(unstruc-convert unstruc)

add_executable(unstruc-offset src/offset.cpp)
target_link_libraries(unstruc-offset unstruc)

add_executable(unstruc-volmesh src/volmesh.cpp)
target_link_libraries(unstruc-volmesh unstruc tet tetmesh)
