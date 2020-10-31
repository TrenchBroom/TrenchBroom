#!/bin/bash

BUILD_DIR=build-measure

if [ -d "$BUILD_DIR" ]; then
	rm -Rf "$BUILD_DIR"
fi 

mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -GNinja .. -DTB_SUPPRESS_PCH=1 -DTB_ENABLE_CCACHE=0

time cmake --build . --target TrenchBroom-nomanual --config Release
