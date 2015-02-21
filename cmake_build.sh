#!/bin/sh

export CC=clang
export CXX=clang++

mkdir -p build/debug
cd build/debug 
cmake -DCMAKE_BUILD_TYPE=DEBUG ../.. 
make $@
cd ../..

mkdir -p build/release 
cd build/release 
cmake -DCMAKE_BUILD_TYPE=RELEASE ../.. 
make $@
cd ../..
