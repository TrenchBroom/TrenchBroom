@echo off
REM msbuild.exe /maxcpucount /p:Configuration=Release /t:Clean
msbuild.exe /maxcpucount /p:Configuration=Release

if %ERRORLEVEL% GEQ 1 goto ERROR

copy TrenchBroom\*.dll Release\
if %ERRORLEVEL% GEQ 1 goto ERROR

cd Release
del TrenchBroom.pdb

For /f "tokens=1,2,3,4,5 delims=/. " %%a in ('date/T') do set CDate=%%c%%b%%a
For /f "tokens=1,2 delims=:" %%f in ('time /t') do set CTime=%%f%%g

7z.exe a "%DROPBOX%\TrenchBroom\TrenchBroom_Win32_%CDATE%_%CTIME%.zip" .\*
if %ERRORLEVEL% GEQ 1 goto ERROR

goto END

:ERROR
echo ===== AN ERROR OCCURED! =====
goto END

:END
pause