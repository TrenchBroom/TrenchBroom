#!/bin/bash

set -o verbose

if [[ $TB_GCC8 == "true" ]] ; then
    export CC=gcc-8
    export CXX=g++-8
else
    export CC=gcc-7
    export CXX=g++-7
fi

# Check versions
/opt/qt59/bin/qmake -v
cmake --version

# Build TB

mkdir build
cd build
cmake .. -GNinja -DCMAKE_PREFIX_PATH=/opt/qt59 -DCMAKE_BUILD_TYPE=Release || exit 1 # FIXME: restore "-DCMAKE_CXX_FLAGS=-Werror"
#cmake --build . --target cppcheck || exit 1
cmake --build . --config Release || exit 1
cpack || exit 1

./generate_checksum_deb.sh
./generate_checksum_rpm.sh

# Run tests (wxgtk needs an X server running for the app to initialize)

xvfb-run -a ./TrenchBroom-Test || exit 1
xvfb-run -a ./TrenchBroom-Benchmark || exit 1

echo "Shared libraries used:"
ldd --verbose ./trenchbroom

echo "Debian dependencies:"
./print_debian_dependencies.sh
