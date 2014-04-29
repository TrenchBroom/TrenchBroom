@echo off

set /p VersionStr=<version
set /p Build=<buildno

for /f "tokens=1,2,3 delims=." %%a in ("%VersionStr%") do set Major=%%a&set Minor=%%b&set Maintenance=%%c
set /A Build+=1

echo #define VERSION "%Major%.%Minor%.%Maintenance% Build %Build%">..\src\Version.h
(echo.|set /p =%Build%)>buildno
