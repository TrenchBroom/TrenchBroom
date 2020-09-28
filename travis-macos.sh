#!/bin/bash

set -o verbose

brew update > /dev/null # Discard stdout, otherwise this spams ~3000 lines to the log
brew install cmake p7zip pandoc cppcheck qt5 ninja

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
cmake .. -GNinja -DCMAKE_BUILD_TYPE="$BUILD_TYPE_VALUE" -DCMAKE_CXX_FLAGS="-Werror -DQT_NO_DEPRECATED_WARNINGS=1" -DTB_ENABLE_ASAN="$TB_ENABLE_ASAN_VALUE" -DTB_RUN_MACDEPLOYQT=1 -DTB_SUPPRESS_PCH=1 -DCMAKE_PREFIX_PATH="$(brew --prefix qt5)" || exit 1

cmake --build . --target cppcheck
if [[ $? -ne 0 ]] ; then
    echo
    echo "cppcheck detected issues, see below"
    echo

    cat common/cppcheck-errors.txt
    echo

    exit 1
fi

cmake --build . --config "$BUILD_TYPE_VALUE" || exit 1

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/vecmath/test"
./vecmath-test || exit 1

cd "$BUILD_DIR/lib/kdl/test"
./kdl-test || exit 1

cd "$BUILD_DIR/common/test"
./common-test || exit 1

if [[ $TB_DEBUG_BUILD != "true" ]] ; then
    cd "$BUILD_DIR/common/benchmark"
    ./common-benchmark || exit 1
else
    echo "Skipping common-benmchark because this is a debug build"
fi

cd "$BUILD_DIR"

cpack || exit 1
./app/generate_checksum.sh

echo "Shared libraries used:"
otool -L ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom
