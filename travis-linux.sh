#!/bin/bash

set -o verbose

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get -qq update
sudo apt-get -y install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc cmake p7zip-full ninja-build xvfb rpm g++-5

# Patch and build wxWidgets

wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxWidgets-3.1.1.7z
if [[ "8d98975eb9f81036261c0643755b98e4bb5ab776" != $(sha1sum wxWidgets-3.1.1.7z | cut -f1 -d' ') ]] ; then exit 1 ; fi
7z x -o"wxWidgets" -y wxWidgets-3.1.1.7z > /dev/null
cd wxWidgets || exit 1
#patch -p0 < ../patches/wxWidgets/*.patch || exit 1
mkdir build-release
cd build-release
CC=gcc-5 CXX=g++-5 ../configure --quiet --disable-shared --with-opengl --with-cxx=14 --with-gtk=2 --prefix=$(pwd)/install --disable-precomp-headers --with-libpng=builtin --with-libtiff=builtin --with-libjpeg=builtin && make -j2 && make install
cd ..
cd ..

# Build TB

mkdir build
cd build
CC=gcc-5 CXX=g++-5 cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-Werror -DwxWidgets_PREFIX=$(pwd)/../wxWidgets/build-release/install || exit 1
cmake --build . --config Release || exit 1
cpack || exit 1

./generate_checksum_deb.sh
./generate_checksum_rpm.sh

# Run tests (wxgtk needs an X server running for the app to initialize)

Xvfb :10 &
export DISPLAY=:10
./TrenchBroom-Test

echo "Shared libraries used:"
ldd --verbose ./trenchbroom

echo "Debian dependencies:"
./print_debian_dependencies.sh
