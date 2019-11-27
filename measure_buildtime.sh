#!/bin/bash

ccache -C

BUILD_DIR=build-measure

if [ -d "$BUILD_DIR" ]; then
	rm -Rf "$BUILD_DIR"
fi 

mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -GNinja .. -DTB_SUPPRESS_PCH=1

time cmake --build . --target TrenchBroom-nomanual --config Release
