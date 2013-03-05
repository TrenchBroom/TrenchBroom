@echo off

set /p VersionStr=<..\..\Source\Version
set /p Build=<BuildNo

for /f "tokens=1,2,3 delims=." %%a in ("%VersionStr%") do set Major=%%a&set Minor=%%b&set Maintenance=%%c
set /A Build+=1

echo %Build%>BuildNo
echo #define VERSION %Major%,%Minor%,%Maintenance%,%Build% > Version.h
echo #define VERSIONSTR "%Major%.%Minor%.%Maintenance% Build %Build%\0" >> Version.h
