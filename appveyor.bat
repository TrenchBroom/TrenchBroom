PATH=%PATH%;C:\Program Files (x86)\Pandoc;C:\Program Files\Cppcheck

REM Check versions
qmake -v
cmake --version
cppcheck --version

mkdir cmakebuild
cd cmakebuild

cmake .. -G"Visual Studio 15 2017" -DCMAKE_PREFIX_PATH="%QT5_INSTALL_DIR%" -T v141 -DCMAKE_BUILD_TYPE=Release

REM  -DCMAKE_CXX_FLAGS=/WX

IF ERRORLEVEL 1 GOTO ERROR

cmake --build . --target cppcheck

IF ERRORLEVEL 1 GOTO ERROR_CPPCHECK

cmake --build . --config Release

IF ERRORLEVEL 1 GOTO ERROR

Release\vecmath\test\vecmath-test.exe
IF ERRORLEVEL 1 GOTO ERROR

Release\common\test\common-test.exe
IF ERRORLEVEL 1 GOTO ERROR

Release\common\benchmark\common-benchmark.exe
IF ERRORLEVEL 1 GOTO ERROR

cpack

IF ERRORLEVEL 1 GOTO ERROR

call generate_checksum.bat

GOTO END

:ERROR_CPPCHECK

echo.
echo "cppcheck detected issues, see below"
echo.

type cppcheck-errors.txt

echo.

:ERROR

echo "Building TrenchBroom failed"
exit /b 1

:END
