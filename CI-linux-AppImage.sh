#!/bin/bash

set -o verbose

# Check versions
qmake -v
cmake --version
pandoc --version

# Build TB

mkdir buildAppImage
cd buildAppImage
cmake .. -DCMAKE_PREFIX_PATH="cmake/packages" -DAPPIMAGE_BUILD=ON -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" -DTB_SUPPRESS_PCH=1 || exit 1
make
make install DESTDIR=AppDir

# TODO: need to be resolved VERSION=??
linuxdeploy-x86_64.AppImage --appdir AppDir -e "AppDir/usr/bin/trenchbroom" \
                                            -d "AppDir/usr/share/applications/trenchbroom.desktop" \
                                            --plugin qt --output appimage
