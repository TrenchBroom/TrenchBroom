@echo off

cd Release
del TrenchBroom.pdb
del TrenchBroom.log

For /f "tokens=1,2,3,4,5 delims=/. " %%a in ('date/T') do set CDate=%%c%%b%%a
For /f "tokens=1,2 delims=:" %%f in ('time /t') do set CTime=%%f%%g

7z.exe a "%DROPBOX%\TrenchBroom\TrenchBroom_Win32_%CDATE%_%CTIME%.zip" .\*

pause
