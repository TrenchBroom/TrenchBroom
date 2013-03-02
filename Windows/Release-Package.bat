@echo off

call Release-Build.bat

if errorlevel 1 goto errorDone

set /p VersionStr=<..\Source\Version
set /p Build=<TrenchBroom\BuildNo

cd Release
del *.pdb
del TrenchBroom.log

7z.exe a "..\..\Release\TrenchBroom_Win32_%VersionStr%_%Build%.zip" .\*
cd ..

goto done

:errorDone

:done

