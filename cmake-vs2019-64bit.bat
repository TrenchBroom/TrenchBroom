REM cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\5.15.2\msvc2015_64" ..
pause