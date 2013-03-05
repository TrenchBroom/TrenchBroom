#!/bin/bash
version=$(<"../Source/Version")
buildNo=$(<"BuildNo")
buildNo=$(($buildNo + 1))
echo $buildNo > "BuildNo"
echo "#define VERSIONSTR" "\"$version Build $buildNo\0\"" > "Version.h"

