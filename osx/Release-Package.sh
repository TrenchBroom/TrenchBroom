#!/bin/bash
xcodebuild

version=$(<../Source/Version)
build=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" TrenchBroom/TrenchBroom-Info.plist)

cd build/Release/
mkdir "../../../Release"
filename="../../../Release/TrenchBroom_Mac_"$version"_"$build".zip"

zip -r "$filename" "./TrenchBroom.app"
