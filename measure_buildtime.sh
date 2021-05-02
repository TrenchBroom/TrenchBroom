#!/bin/bash

BUILD_DIR=build-measure

if [ -d "$BUILD_DIR" ]; then
	rm -Rf "$BUILD_DIR"
fi 

mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -GNinja .. -DTB_SUPPRESS_PCH=1 -DTB_ENABLE_CCACHE=0 -DCMAKE_PREFIX_PATH=/Users/kristian/Qt/5.15.2/clang_64/lib/cmake/Qt5

time cmake --build . --config Release
