#!/bin/bash

set -o verbose

brew update
brew install cmake p7zip pandoc cppcheck qt5

# Sometimes homebrew complains that cmake is already installed, but we need the latest version.
brew upgrade cmake

# Check versions
qmake -v
cmake --version
cppcheck --version

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
cmake .. -GXcode -DTB_ENABLE_ASAN="$TB_ENABLE_ASAN_VALUE" -DTB_RUN_MACDEPLOYQT=1 -DCMAKE_PREFIX_PATH="$(brew --prefix qt5)" || exit 1 # FIXME: Restore -DCMAKE_CXX_FLAGS="-Werror"

cmake --build . --target cppcheck
if [[ $? -ne 0 ]] ; then
    echo
    echo "cppcheck detected issues, see below"
    echo

    cat cppcheck-errors.txt
    echo

    exit 1
fi

cmake --build . --config "$BUILD_TYPE_VALUE" -- -quiet || exit 1

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/vecmath/test/$BUILD_TYPE_VALUE"
./vecmath-test || exit 1

cd "$BUILD_DIR/common/test/$BUILD_TYPE_VALUE"
./common-test || exit 1

cd "$BUILD_DIR/common/benchmark/$BUILD_TYPE_VALUE"
./common-benchmark || exit 1

cd "$BUILD_DIR"

cpack -C $BUILD_TYPE_VALUE || exit 1
./app/generate_checksum.sh

echo "Shared libraries used:"
otool -L ./app/$BUILD_TYPE_VALUE/TrenchBroom.app/Contents/MacOS/TrenchBroom
