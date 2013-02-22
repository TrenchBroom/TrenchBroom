@echo off

set /p Version=<Version.txt

for /f "tokens=1,2,3,4 delims=." %%a in ("%Version%") do set Major=%%a&set Minor=%%b&set Maintenance=%%c&set Build=%%d
set /A Build+=1

echo %Major%.%Minor%.%Maintenance%.%Build% > Version.txt
echo #define VERSION %Major%,%Minor%,%Maintenance%,%Build% > Version.h
echo #define VERSIONSTR "%Major%.%Minor%.%Maintenance%.%Build%\0" >> Version.h
