REM Check versions
qmake -v
cmake --version
ninja --version
ccache --version
pandoc --version
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

REM CCache configuration
ccache -p

mkdir cmakebuild
cd cmakebuild

REM Treat warnings as errors
REM Don't pass -DCMAKE_CXX_FLAGS="/WX" on the cmake command line; doing so wipes out necessary cmake-provided defaults such as "/EHsc"
set CXXFLAGS="/WX"

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake .. -GNinja -DCMAKE_PREFIX_PATH="%QT_ROOT_DIR%" -DCMAKE_BUILD_TYPE=Release -DTB_ENABLE_PCH=0 -DTB_ENABLE_CCACHE=1 -DTB_RUN_WINDEPLOYQT=1

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ccache -z
cmake --build . --config Release

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ccache -s

set BUILD_DIR="%cd%"

cd lib\vm\test
vm-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd lib\kdl\test
kdl-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd lib\upd\test
upd-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd common\test
common-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

common-regression-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd common\benchmark
common-benchmark.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

"C:\Program Files\CMake\bin\cpack.exe"

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

call generate_checksum.bat

GOTO END

:ERROR

echo "Building TrenchBroom failed"
exit /b 1

:END
