REM Environment variables required before running this build script:
REM   QT5_INSTALL_DIR - set to the Qt directory, e.g. "set QT5_INSTALL_DIR=D:\Qt\5.12.9\msvc2017_64"
REM   TB_VS_DIR - VS directory, e.g "set TB_VS_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
REM   TB_ARCH - "set TB_ARCH=x64" or "set TB_ARCH=x86"

echo Build script settings:
echo QT5_INSTALL_DIR: %QT5_INSTALL_DIR%
echo TB_VS_DIR: %TB_VS_DIR%
echo TB_ARCH: %TB_ARCH%

REM Setup Visual Studio environment variables for cmake/ninja
REM See: https://stackoverflow.com/a/31589526
REM
REM We need a 142 (VS2019) toolset, doesn't need to be this version in particular
REM but might as well pin a specific version
call "%TB_VS_DIR%\VC\Auxiliary\Build\vcvarsall.bat" %TB_ARCH% -vcvars_ver=14.28
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

PATH=%PATH%;C:\Program Files (x86)\Pandoc

REM Init submodules
git submodule update --init --recursive

REM Check versions
qmake -v
cmake --version
pandoc --version

mkdir cmakebuild
cd cmakebuild

REM Treat warnings as errors
REM Don't pass -DCMAKE_CXX_FLAGS="/WX" on the cmake command line; doing so wipes out necessary cmake-provided defaults such as "/EHsc"
set CXXFLAGS="/WX"

cmake .. -GNinja -DCMAKE_PREFIX_PATH="%QT5_INSTALL_DIR%" -DCMAKE_BUILD_TYPE=Release -DTB_SUPPRESS_PCH=1

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
