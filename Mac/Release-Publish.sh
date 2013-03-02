#!/bin/bash
xcodebuild

cd build/Release/

_TB_NOW=$(date +"%Y%m%d_%H%M")
zip -r $DROPBOX/TrenchBroom/TrenchBroom_Mac_$_TB_NOW ./TrenchBroom.app
