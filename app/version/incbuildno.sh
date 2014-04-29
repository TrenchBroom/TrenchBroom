#!/bin/bash
version=$(<"version")
buildNo=$(<"buildno")
buildNo=${buildNo//[[:blank:]]/}
echo ${buildNo}
buildNo=$(($buildNo + 1))
echo "#define VERSION" "\"$version Build $buildNo\"" > ../src/Version.h
echo $buildNo > buildno
