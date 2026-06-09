#!/bin/bash

# set -o verbose

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Check versions
cmake --version
ninja --version
pandoc --version

# Build TB

rm -rf cmakebuild
mkdir cmakebuild
cd cmakebuild

# AppImage tools (linuxdeploy and plugins) may need extract-and-run mode in
# containers where FUSE mounts are unavailable.
export APPIMAGE_EXTRACT_AND_RUN=1

# Catch2 test discovery runs test binaries during the build. Ensure a UTF-8
# locale and headless Qt platform are set for both build-time discovery and
# ctest execution.
export LANG=C.UTF-8
export LC_ALL=C.UTF-8
export QT_QPA_PLATFORM=offscreen

# Normalize ccache keys across ephemeral CI containers and checkout roots.
export CCACHE_BASEDIR="${CCACHE_BASEDIR:-$SCRIPT_DIR}"
export CCACHE_NOHASHDIR=1
export CCACHE_COMPILERCHECK=content

# install linuxdeploy into the build dir so it gets cleared with it
wget -nc https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -nc https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod u+x ./linuxdeploy-x86_64.AppImage
chmod u+x ./linuxdeploy-plugin-qt-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --version
./linuxdeploy-plugin-qt-x86_64.AppImage --plugin-version

cmake .. \
  -DCMAKE_PREFIX_PATH="cmake/packages;$QT_ROOT_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-Werror" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" \
  -DTB_ENABLE_CCACHE=1 \
  -DTB_ENABLE_PCH=0 \
  -DCMAKE_INSTALL_PREFIX=/usr \
  || exit 1

cmake --build . --config Release -- -j $(nproc) || exit 1

# Run tests

BUILD_DIR=$(pwd)
ctest --test-dir "$BUILD_DIR" --output-on-failure -j || exit 1

cd "$BUILD_DIR"
ldd --verbose ./app/TrenchBroom/trenchbroom

cpack || exit 1
./app/TrenchBroom/generate_checksum.sh
