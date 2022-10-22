#!/bin/bash

set -o verbose

# Check versions
qmake -v
cmake --version
pandoc --version

# Build TB

mkdir build
cd build
cmake .. -GNinja -DCMAKE_PREFIX_PATH="cmake/packages" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" -DTB_SUPPRESS_PCH=1 || exit 1
cmake --build . --config Release || exit 1

# Run tests (wxgtk needs an X server running for the app to initialize)

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/vecmath/test"
./vecmath-test || exit 1

cd "$BUILD_DIR/lib/kdl/test"
./kdl-test || exit 1

cd "$BUILD_DIR/common/test"
xvfb-run -a ./common-test || exit 1
xvfb-run -a ./common-regression-test || exit 1

if [[ $TB_DEBUG_BUILD != "true" ]] ; then
    cd "$BUILD_DIR/common/benchmark"
    xvfb-run -a ./common-benchmark || exit 1
else
    echo "Skipping common-benmchark because this is a debug build"
fi


cd "$BUILD_DIR"

cpack || exit 1

./app/generate_checksum_deb.sh
./app/generate_checksum_rpm.sh

echo "Shared libraries used:"
ldd --verbose ./app/trenchbroom

./app/print_linux_package_info.sh
