#!/bin/bash

set -o verbose

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get -qq update
sudo apt-get -y install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc cmake p7zip-full ninja-build xvfb rpm

if [[ $TB_GCC8 == "true" ]] ; then
    export CC=gcc-8
    export CXX=g++-8
    sudo apt-get -y install g++-8
else
    export CC=gcc-7
    export CXX=g++-7
    sudo apt-get -y install g++-7
fi

# Patch and build wxWidgets

export WX_CACHE_FULLPATH="${TRAVIS_BUILD_DIR}/wx-install-cache"

if [[ ! -e wx-install-cache/bin/wx-config ]]; then
    echo "wxwidgets cache directory invalid. Building wxwidgets..."

    wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxWidgets-3.1.1.7z
    if [[ "8d98975eb9f81036261c0643755b98e4bb5ab776" != $(sha1sum wxWidgets-3.1.1.7z | cut -f1 -d' ') ]] ; then exit 1 ; fi
    7z x -o"wxWidgets" -y wxWidgets-3.1.1.7z > /dev/null
    cd wxWidgets || exit 1
    #patch -p0 < ../patches/wxWidgets/*.patch || exit 1
    mkdir build-release
    cd build-release
    ../configure --quiet --disable-shared --with-opengl --with-cxx=17 --with-gtk=2 --prefix=$WX_CACHE_FULLPATH --disable-precomp-headers --with-libpng=builtin --with-libtiff=builtin --with-libjpeg=builtin && make -j2 && make install
    cd ..
    cd ..
else
    echo "using cached copy of wxwidgets"
fi

# Build TB

mkdir build
cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-Werror -DwxWidgets_PREFIX=$WX_CACHE_FULLPATH || exit 1
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
