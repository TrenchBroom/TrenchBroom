PATH=%PATH%;C:\Program Files (x86)\Pandoc;C:\Program Files\Cppcheck

mkdir cmakebuild
cd cmakebuild

cmake .. -G"Visual Studio 15 2017" -T v141_xp -DCMAKE_BUILD_TYPE=Release -DTB_SKIP_TESTS=YES -DCMAKE_PREFIX_PATH="C:\Qt\5.12.1\msvc2017"

REM  -DCMAKE_CXX_FLAGS=/WX

IF ERRORLEVEL 1 GOTO ERROR

cmake --build . --target cppcheck

IF ERRORLEVEL 1 GOTO ERROR

cmake --build . --config Release

IF ERRORLEVEL 1 GOTO ERROR

cpack

IF ERRORLEVEL 1 GOTO ERROR

call generate_checksum.bat

Release\TrenchBroom-Test.exe

IF ERRORLEVEL 1 GOTO ERROR

Release\TrenchBroom-Benchmark.exe

IF ERRORLEVEL 1 GOTO ERROR

GOTO END

:ERROR

echo "Building TrenchBroom failed"

:END
