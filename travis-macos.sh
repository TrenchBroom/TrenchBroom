#!/bin/bash

set -o verbose

brew update
brew install cmake p7zip pandoc

# Patch and build wxWidgets

export WX_CACHE_FULLPATH="${TRAVIS_BUILD_DIR}/wx-install-cache"

if [[ ! -e wx-install-cache/bin/wx-config ]]; then
    echo "wxwidgets cache directory invalid. Building wxwidgets..."

    wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxWidgets-3.1.1.7z
    if [[ "8d98975eb9f81036261c0643755b98e4bb5ab776" != $(openssl sha1 wxWidgets-3.1.1.7z | cut -f2 -d' ') ]] ; then exit 1 ; fi
    7z x -o"wxWidgets" -y wxWidgets-3.1.1.7z > /dev/null
    cd wxWidgets || exit 1
    cat ../patches/wxWidgets/*.patch | patch -p0 || exit 1
    mkdir build-release
    cd build-release
    ../configure --quiet --with-osx_cocoa --disable-shared --disable-mediactrl --with-opengl --with-macosx-version-min=10.9 --with-cxx=17 --prefix=$WX_CACHE_FULLPATH --disable-precomp-headers --with-libpng=builtin --without-libtiff --with-libjpeg=builtin && make -j2 && make install
    cd ..
    cd ..
else
    echo "using cached copy of wxwidgets"
fi

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
cmake .. -GXcode -DCMAKE_BUILD_TYPE="$BUILD_TYPE_VALUE" -DCMAKE_CXX_FLAGS="-Werror" -DTB_ENABLE_ASAN="$TB_ENABLE_ASAN_VALUE" -DwxWidgets_PREFIX=$WX_CACHE_FULLPATH || exit 1
cmake --build . --config "$BUILD_TYPE_VALUE" || exit 1
cpack -C $BUILD_TYPE_VALUE || exit 1

./generate_checksum.sh

cd "$BUILD_TYPE_VALUE" 
./TrenchBroom-Test || exit 1
./TrenchBroom-Benchmark || exit 1

echo "Shared libraries used:"
otool -L ./TrenchBroom.app/Contents/MacOS/TrenchBroom
