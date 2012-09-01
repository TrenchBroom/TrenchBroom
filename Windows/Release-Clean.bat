@echo off
msbuild.exe /maxcpucount /p:Configuration=Release /t:Clean
rmdir /S /Q Release
pause