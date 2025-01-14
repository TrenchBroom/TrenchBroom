#!/bin/bash

set -o verbose

# Check versions
qmake -v
cmake --version
pandoc --version

# Qt install prefix
brew --prefix qt@6

# Build TB

TB_BUILD_TYPE="Release"
TB_ENABLE_ASAN=0

# Note: When this variable is changed, vcpkg will need to recompile all dependencies.
# However, vcpkg will not detect the change and will happily keep using any cached binaries (see
# the lukka/run-vcpkg workflow step for details). This will cause a mismatch between the deployment
# target under which the binaries were compiled and the new deployment target used here.
# Therefore, when this variable is changed, the vcpkg binary cache must be invalidated. The easiest
# way to do that is to update vcpkg to the latest version because the vcpkg commit ID is part of the
# cache key for the binary cache.
export MACOSX_DEPLOYMENT_TARGET=10.15

if [[ $TB_DEBUG_BUILD == "true" ]] ; then
    TB_BUILD_TYPE="Debug"
    TB_ENABLE_ASAN=1
fi

echo "Build type: $TB_BUILD_TYPE"
echo "TB_ENABLE_ASAN: $TB_ENABLE_ASAN"

mkdir cmakebuild
cd cmakebuild
cmake .. -GNinja -DCMAKE_BUILD_TYPE="$TB_BUILD_TYPE" -DCMAKE_CXX_FLAGS="-Werror" -DCMAKE_EXE_LINKER_FLAGS="-Wl,-fatal_warnings" -DTB_ENABLE_ASAN="$TB_ENABLE_ASAN" -DTB_RUN_MACDEPLOYQT=1 -DTB_ENABLE_PCH=0 -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" || exit 1

cmake --build . --config "$TB_BUILD_TYPE" || exit 1

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/vm/test"
./vm-test || exit 1

cd "$BUILD_DIR/lib/kdl/test"
./kdl-test || exit 1

cd "$BUILD_DIR/common/test"
./common-test || exit 1
./common-regression-test || exit 1

if [[ $TB_DEBUG_BUILD != "true" ]] ; then
    cd "$BUILD_DIR/common/benchmark"
    ./common-benchmark || exit 1
else
    echo "Skipping common-benmchark because this is a debug build"
fi

cd "$BUILD_DIR"

# see https://github.com/actions/runner-images/issues/7522
echo killing...; sudo pkill -9 XProtect >/dev/null || true;
echo waiting...; while pgrep XProtect; do sleep 3; done;

# retry up to 3 times to work around hdiutil failing to create a DMG image
retry --tries=3 --fail="exit 1" cpack
./app/generate_checksum.sh

echo "Deployment target (minos):"
otool -l ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom | grep minos

echo "Shared libraries used:"
otool -L ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom

echo "Binary type:"
file ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom
