@echo off
msbuild.exe /maxcpucount /p:Configuration=Release /t:Clean
pause