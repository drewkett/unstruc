# Set  the minimum  required version  of cmake  for a  project.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_COMPILE_OPTIONS(--std=gnu++11)
PROJECT(unstruc)

IF(DEFINED BUILD_SUFFIX)
	set(CMAKE_EXECUTABLE_SUFFIX ${BUILD_SUFFIX})
ENDIF(DEFINED BUILD_SUFFIX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

ADD_SUBDIRECTORY(src/unstruc)
ADD_SUBDIRECTORY(src/tetmesh)
ADD_SUBDIRECTORY(src/tetgen1.5.0)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/tetgen1.5.0)

add_executable(unstruc-convert src/convert.cpp)
target_link_libraries(unstruc-convert unstruc)

add_executable(unstruc-offset src/offset.cpp)
target_link_libraries(unstruc-offset unstruc tet tetmesh)

add_executable(unstruc-quality src/quality.cpp)
target_link_libraries(unstruc-quality unstruc)
