#!/bin/bash

# set -o verbose

# Check versions
qmake -v
cmake --version
ninja --version
pandoc --version

# Qt install prefix
brew --prefix qt@6

# Note: When this variable is changed, vcpkg will need to recompile all dependencies.
# However, vcpkg will not detect the change and will happily keep using any cached
# binaries (see the lukka/run-vcpkg workflow step for details). This will cause a mismatch
# between the deployment target under which the binaries were compiled and the new
# deployment target used here. Therefore, when this variable is changed, the vcpkg binary
# cache must be invalidated. The easiest way to do that is to update vcpkg to the latest
# version because the vcpkg commit ID is part of the cache key for the binary cache.
export MACOSX_DEPLOYMENT_TARGET=12.0

# Build TB

TB_BUILD_TYPE="Release"
if [[ $TB_ENABLE_ASAN == "true" ]] ; then
    TB_BUILD_TYPE="Debug"
fi

echo "TB_BUILD_TYPE: $TB_BUILD_TYPE"
echo "TB_ENABLE_ASAN: $TB_ENABLE_ASAN"
echo "TB_SIGN_MAC_BUNDLE: $TB_SIGN_MAC_BUNDLE"

# Note: The app bundle and the archive should be signed and notarized, otherwise macOS'
# gatekeeper will refuse to run the app. The app itself is signed as a post-build step
# using macdeployqt, and the archive is signed and notarized by the
# ./app/sign_macos_archive.sh script that is called when the build was successful. This
# script is generated from the cmake template SignMacOsBundle.cmake.in. See Build.md for
# more details on how to set up the necessary prerequisites for signing and notarizing the
# app and the archive.


mkdir cmakebuild
cd cmakebuild
cmake .. \
  -GNinja \
  -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" \
  -DCMAKE_BUILD_TYPE="$TB_BUILD_TYPE" \
  -DCMAKE_CXX_FLAGS="-Werror" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,-fatal_warnings" \
  -DTB_ENABLE_CCACHE=0 \
  -DTB_ENABLE_PCH=0 \
  -DTB_ENABLE_ASAN="$TB_ENABLE_ASAN" \
  -DTB_RUN_MACDEPLOYQT=1 \
  -DTB_SIGN_MAC_BUNDLE=$TB_SIGN_MAC_BUNDLE \
  -DTB_SIGN_IDENTITY="$TB_SIGN_IDENTITY" \
  -DTB_NOTARIZATION_EMAIL="$TB_NOTARIZATION_EMAIL" \
  -DTB_NOTARIZATION_TEAM_ID="$TB_NOTARIZATION_TEAM_ID" \
  -DTB_NOTARIZATION_PASSWORD="$TB_NOTARIZATION_PASSWORD" \
  || exit 1

cmake --build . --config "$TB_BUILD_TYPE" || exit 1

BUILD_DIR=$(pwd)

cd "$BUILD_DIR/lib/VmLib/test"
./VmLibTest || exit 1

cd "$BUILD_DIR/lib/KdLib/test"
./KdLibTest || exit 1

cd "$BUILD_DIR/lib/upd/test"
./upd-test || exit 1

cd "$BUILD_DIR/common/test"
./common-test || exit 1

if [[ $TB_ENABLE_ASAN == "false" ]] ; then
  cd "$BUILD_DIR"

  # see https://github.com/actions/runner-images/issues/7522
  echo killing...; sudo pkill -9 XProtect >/dev/null || true;
  echo waiting...; while pgrep XProtect; do sleep 3; done;

  cpack || exit 1
  ./app/sign_macos_archive.sh || exit 1
  ./app/generate_checksum.sh || exit 1

  echo "Deployment target (minos):"
  otool -l ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom | grep minos

  echo "Shared libraries used:"
  otool -L ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom

  echo "Binary type:"
  file ./app/TrenchBroom.app/Contents/MacOS/TrenchBroom
else
    echo "Skipping packaging because this is an ASAN build"
fi
