#!/bin/bash

set -o verbose

brew update
brew install cmake p7zip pandoc cppcheck qt5

# Check Qt version
qmake -v

# Build TB

BUILD_TYPE_VALUE="Release"
TB_ENABLE_ASAN_VALUE="NO"

if [[ $TB_DEBUG_BUILD == "true" ]] ; then
    BUILD_TYPE_VALUE="Debug"
    TB_ENABLE_ASAN_VALUE="YES"
fi

echo "Build type: $BUILD_TYPE_VALUE"
echo "TB_ENABLE_ASAN: $TB_ENABLE_ASAN_VALUE"

mkdir build
cd build
cmake .. -GXcode -DCMAKE_BUILD_TYPE="$BUILD_TYPE_VALUE" -DTB_ENABLE_ASAN="$TB_ENABLE_ASAN_VALUE" -DTB_RUN_MACDEPLOYQT=1 -DCMAKE_PREFIX_PATH="$(brew --prefix qt5)" || exit 1 # FIXME: Restore -DCMAKE_CXX_FLAGS="-Werror"
cmake --build . --target cppcheck || exit 1
cmake --build . --config "$BUILD_TYPE_VALUE" || exit 1
cpack -C $BUILD_TYPE_VALUE || exit 1

./generate_checksum.sh

cd "$BUILD_TYPE_VALUE" 
./TrenchBroom-Test || exit 1
./TrenchBroom-Benchmark || exit 1

echo "Shared libraries used:"
otool -L ./TrenchBroom.app/Contents/MacOS/TrenchBroom
