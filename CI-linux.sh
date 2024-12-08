#!/bin/bash

set -o verbose

# install linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20240109-1/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20240109-1/linuxdeploy-plugin-qt-x86_64.AppImage
chmod u+x ./linuxdeploy-x86_64.AppImage
chmod u+x ./linuxdeploy-plugin-qt-x86_64.AppImage

# Check versions
qmake -v
cmake --version
pandoc --version
./linuxdeploy-x86_64.AppImage --version
./linuxdeploy-plugin-qt-x86_64.AppImage --version

# Build TB

mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH="cmake/packages" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" -DTB_SUPPRESS_PCH=1 || exit 1
cmake --build . --config Release -- -j $(nproc) || exit 1

# Run tests (wxgtk needs an X server running for the app to initialize)

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/vm/test"
./vm-test || exit 1

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

-ldd --verbose ./app/trenchbroom

make install DESTDIR=AppDir | exit 1

../linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage --desktop-file ../app/resources/linux/trenchbroom.desktop --icon-file ../app/resources/graphics/images/AppIcon.png --icon-filename trenchbroom --plugin qt

cmake -E md5sum TrenchBroom-x86_64.AppImage > TrenchBroom-x86_64.AppImage.md5
