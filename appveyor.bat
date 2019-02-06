PATH=%PATH%;C:\Program Files (x86)\Pandoc

mkdir cmakebuild
cd cmakebuild

cmake .. -G"Visual Studio 15 2017" -T v141_xp -DCMAKE_BUILD_TYPE=Release -DTB_SKIP_TESTS=YES -DCMAKE_PREFIX_PATH="C:\Qt\5.11.2\msvc2015"

REM  -DCMAKE_CXX_FLAGS=/WX

IF ERRORLEVEL 1 GOTO ERROR

cmake --build . --config Release

IF ERRORLEVEL 1 GOTO ERROR

cpack

IF ERRORLEVEL 1 GOTO ERROR

call generate_checksum.bat

REM Release\TrenchBroom-Test.exe

REM IF ERRORLEVEL 1 GOTO ERROR

REM Release\TrenchBroom-Benchmark.exe

REM IF ERRORLEVEL 1 GOTO ERROR

GOTO END

:ERROR

echo "Building TrenchBroom failed"

:END
