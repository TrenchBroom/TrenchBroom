@echo off
msbuild.exe /maxcpucount /p:Configuration=Release

copy TrenchBroom\*.dll Release\

pause