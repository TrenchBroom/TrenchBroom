#!/bin/bash

# set -o verbose

# install linuxdeploy
wget -nc https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20240109-1/linuxdeploy-x86_64.AppImage
wget -nc https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20240109-1/linuxdeploy-plugin-qt-x86_64.AppImage
chmod u+x ./linuxdeploy-x86_64.AppImage
chmod u+x ./linuxdeploy-plugin-qt-x86_64.AppImage

# Check versions
qmake -v
cmake --version
ninja --version
pandoc --version
./linuxdeploy-x86_64.AppImage --version
./linuxdeploy-plugin-qt-x86_64.AppImage --plugin-version

# Build TB

mkdir cmakebuild
cd cmakebuild
cmake .. \
  -DCMAKE_PREFIX_PATH="cmake/packages;$QT_ROOT_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-Werror" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" \
  -DTB_ENABLE_CCACHE=0 \
  -DTB_ENABLE_PCH=0 \
  -DCMAKE_INSTALL_PREFIX=/usr \
  || exit 1

cmake --build . --config Release -- -j $(nproc) || exit 1

# Run tests (wxgtk needs an X server running for the app to initialize)

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/VmLib/test"
./VmLibTest || exit 1

cd "$BUILD_DIR/lib/KdLib/test"
./KdLibTest || exit 1

cd "$BUILD_DIR/lib/UpdateLib/test"
./UpdateLibTest || exit 1

cd "$BUILD_DIR/common/test"
xvfb-run -a ./common-test || exit 1

cd "$BUILD_DIR"

ldd --verbose ./app/trenchbroom

cpack || exit 1
./app/generate_checksum.sh
