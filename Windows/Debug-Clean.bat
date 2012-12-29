@echo off
msbuild.exe /maxcpucount /p:Configuration=Debug /t:Clean
rmdir /S /Q Debug
pause