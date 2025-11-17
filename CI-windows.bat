REM Check versions
qmake -v
cmake --version
ninja --version
pandoc --version
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

mkdir cmakebuild
cd cmakebuild

REM Treat warnings as errors
REM Don't pass -DCMAKE_CXX_FLAGS="/WX" on the cmake command line; doing so wipes out necessary cmake-provided defaults such as "/EHsc"
set CXXFLAGS="/WX"

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake .. -GNinja -DCMAKE_PREFIX_PATH="%QT_ROOT_DIR%" -DCMAKE_BUILD_TYPE=Release -DTB_ENABLE_PCH=0 -DTB_ENABLE_CCACHE=0

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

cmake --build . --config Release

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

set BUILD_DIR="%cd%"

cd lib\vm\test
vm-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd lib\KdLib\test
KdLibTest.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd lib\upd\test
upd-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd common\test
common-test.exe
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
