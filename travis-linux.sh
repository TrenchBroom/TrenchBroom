#!/bin/bash

set -o verbose

sudo apt-get -qq update
sudo apt-get -y install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc cmake p7zip-full ninja-build xvfb rpm

# Patch and build wxWidgets

wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.0/wxWidgets-3.1.0.7z
if [[ "daf03ed0006e41334f10ceeb3aa2d20c63aacd42" != $(sha1sum wxWidgets-3.1.0.7z | cut -f1 -d' ') ]] ; then exit 1 ; fi
7z x -o"wxWidgets" -y wxWidgets-3.1.0.7z > /dev/null
cd wxWidgets || exit 1
patch -p0 < ../patches/wxWidgets/*.patch || exit 1
mkdir build-release
cd build-release
../configure --quiet --disable-shared --with-opengl --with-cxx=11 --with-gtk=2 --prefix=$(pwd)/install --disable-precomp-headers && make -j2 && make install
cd ..
cd ..

# Build TB

mkdir build
cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS=-Werror -DwxWidgets_PREFIX=$(pwd)/../wxWidgets/build-release/install || exit 1
ninja || exit 1
cpack || exit 1

# Run tests (wxgtk needs an X server running for the app to initialize)

Xvfb :10 &
export DISPLAY=:10
./TrenchBroom-Test
