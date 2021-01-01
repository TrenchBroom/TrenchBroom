REM Check versions
qmake -v
cmake --version
pandoc --version
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

mkdir cmakebuild
cd cmakebuild

REM Treat warnings as errors
REM Don't pass -DCMAKE_CXX_FLAGS="/WX" on the cmake command line; doing so wipes out necessary cmake-provided defaults such as "/EHsc"
set CXXFLAGS="/WX"

cmake .. -G"Visual Studio 16 2019" -T v142 -A "%TB_ARCH%" -DCMAKE_PREFIX_PATH="%QT5_INSTALL_DIR%" -DCMAKE_BUILD_TYPE=Release -DTB_SUPPRESS_PCH=1

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

cmake --build . --config Release

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

set BUILD_DIR="%cd%"

cd lib\vecmath\test\Release
vecmath-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd lib\kdl\test\Release
kdl-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd common\test\Release
common-test.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cd common\benchmark\Release
common-benchmark.exe
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
cd "%BUILD_DIR%"

cpack

IF %ERRORLEVEL% NEQ 0 GOTO ERROR

call generate_checksum.bat

GOTO END

:ERROR

echo "Building TrenchBroom failed"
exit /b 1

:END
