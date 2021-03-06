# Set  the minimum  required version  of cmake  for a  project.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(unstruc)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  ADD_COMPILE_OPTIONS(--std=gnu++11)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  ADD_COMPILE_OPTIONS(--std=c++11)
endif()

IF(DEFINED BUILD_SUFFIX)
	set(CMAKE_EXECUTABLE_SUFFIX ${BUILD_SUFFIX})
ENDIF(DEFINED BUILD_SUFFIX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

ADD_SUBDIRECTORY(src/unstruc)
ADD_SUBDIRECTORY(src/tetmesh)
ADD_SUBDIRECTORY(src/tetgen1.5.0)

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/tetgen1.5.0)

add_executable(unstruc-convert src/convert.cpp)
target_link_libraries(unstruc-convert unstruc cgns)

add_executable(unstruc-offset src/offset.cpp)
target_link_libraries(unstruc-offset unstruc tet tetmesh cgns)

add_executable(unstruc-quality src/quality.cpp)
target_link_libraries(unstruc-quality unstruc cgns)
