#!/bin/sh

export CC=clang
export CXX=clang++

git tag | grep "^$1$"  || exit 1
git checkout $1 || exit 1
mkdir -p build/version/$1 
cd build/version/$1 
cmake -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_SUFFIX=_$1 ../../..
make -j4
cd ../../..
