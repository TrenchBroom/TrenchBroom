#!/bin/bash

set -o verbose

if [[ $TB_GCC8 == "true" ]] ; then
    export CC=gcc-8
    export CXX=g++-8
else
    export CC=gcc-7
    export CXX=g++-7
fi

# so CPack finds Qt
export LD_LIBRARY_PATH=/opt/qt59/lib:${LD_LIBRARY_PATH}
export PATH=/opt/qt59/bin:${PATH}

# Check versions
qmake -v
cmake --version
pandoc --version

# Build TB

mkdir build
cd build
cmake .. -GNinja -DCMAKE_PREFIX_PATH=/opt/qt59 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DTB_SUPPRESS_PCH=1 || exit 1
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
