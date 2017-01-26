PATH=%PATH%;c:\wxWidgets-3.1.0\lib\vc140_dll
PATH=%PATH%;C:\Program Files (x86)\Pandoc
SET WXWIN="c:\wxWidgets-3.1.0"

mkdir cmakebuild
cd cmakebuild

cmake .. -G"Visual Studio 14 2015" -T v140_xp -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=/WX
msbuild PACKAGE.vcxproj /p:Configuration=Release /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"

IF ERRORLEVEL 1 GOTO ERROR

call generate_checksum.bat

Release\TrenchBroom-Test.exe

GOTO END

:ERROR

echo "Building TrenchBroom failed"

:END
