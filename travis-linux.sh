#!/bin/bash

set -o verbose

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get -qq update
sudo apt-get -y install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc cmake p7zip-full ninja-build xvfb rpm

if [[ $TB_GCC8 == "true" ]] ; then
    export CC=gcc-8
    export CXX=g++-8
    sudo apt-get -y install g++-8
elif [[ $TB_CLANG_LINUX == "true" ]] ; then
    # see http://apt.llvm.org
    # and https://blog.kowalczyk.info/article/k/how-to-install-latest-clang-6.0-on-ubuntu-16.04-xenial-wsl.html
    sudo apt-key add ci/llvm-snapshot.gpg.key
    sudo add-apt-repository -y "deb https://apt.llvm.org/trusty/ llvm-toolchain-trusty-6.0 main"
    sudo apt-get update
    export CC=clang-6.0
    export CXX=clang++-6.0
    sudo apt-get -y install clang-6.0

    clang++-6.0 -v
else
    export CC=gcc-7
    export CXX=g++-7
    sudo apt-get -y install g++-7
fi

# Patch and build wxWidgets

echo "initial cache contents:"
ls wx-install-cache
echo "bin subdir:"
ls wx-install-cache/bin
echo "pwd:"
pwd
echo "wx-config"
ls -al wx-install-cache/bin/wx-config
echo "cat"
cat wx-install-cache/bin/wx-config

export WX_CACHE_FULLPATH="${TRAVIS_BUILD_DIR}/wx-install-cache"

echo "full cache path: $WX_CACHE_FULLPATH"

if [[ ! -e wx-install-cache/bin/wx-config ]]; then
    echo "wxwidgets cache directory invalid. Building wxwidgets..."

    wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxWidgets-3.1.1.7z
    if [[ "8d98975eb9f81036261c0643755b98e4bb5ab776" != $(sha1sum wxWidgets-3.1.1.7z | cut -f1 -d' ') ]] ; then exit 1 ; fi
    7z x -o"wxWidgets" -y wxWidgets-3.1.1.7z > /dev/null
    cd wxWidgets || exit 1
    #patch -p0 < ../patches/wxWidgets/*.patch || exit 1
    mkdir build-release
    cd build-release
    ../configure --quiet --disable-shared --with-opengl --with-cxx=17 --with-gtk=2 --prefix=$WX_CACHE_FULLPATH --disable-precomp-headers --with-libpng=builtin --with-libtiff=builtin --with-libjpeg=builtin && make -j2 && make -v install
    cd ..
    cd ..
else
    echo "using cached copy of wxwidgets"
fi

echo "cache should be valid now:"
ls wx-install-cache
echo "bin subdir:"
ls wx-install-cache/bin
echo "wx-config"
ls -al wx-install-cache/bin/wx-config
echo "cat"
cat wx-install-cache/bin/wx-config

if [[ ! -e wx-install-cache/bin/wx-config ]]; then
    echo "wxwidgets cache directory would be valid."
else
    echo "wxwidgets cache directory would be invalid."
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

Xvfb :10 &
export DISPLAY=:10
./TrenchBroom-Test || exit 1
./TrenchBroom-Benchmark || exit 1

echo "Shared libraries used:"
ldd --verbose ./trenchbroom

echo "Debian dependencies:"
./print_debian_dependencies.sh
