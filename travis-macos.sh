#!/bin/bash

set -o verbose

brew update
brew install cmake ninja p7zip pandoc

# Patch and build wxWidgets

wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.0/wxWidgets-3.1.0.7z
if [[ "daf03ed0006e41334f10ceeb3aa2d20c63aacd42" != $(openssl sha1 wxWidgets-3.1.0.7z | cut -f2 -d' ') ]] ; then exit 1 ; fi
7z x -o"wxWidgets" -y wxWidgets-3.1.0.7z > /dev/null
cd wxWidgets || exit 1
patch -p0 < ../patches/wxWidgets/*.patch || exit 1
mkdir build-release
cd build-release
../configure --quiet --with-osx_cocoa --disable-shared --disable-mediactrl --with-opengl --with-macosx-version-min=10.9 --with-cxx=11 --prefix=$(pwd)/install --disable-precomp-headers && make -j2 && make install
cd ..
cd ..

# Build TB

mkdir build
cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS=-Werror -DwxWidgets_PREFIX=$(pwd)/../wxWidgets/build-release/install || exit 1
ninja || exit 1
cpack || exit 1
./TrenchBroom-Test
