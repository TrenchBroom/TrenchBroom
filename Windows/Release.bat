@echo off
REM c:\Windows\Microsoft.Net\Framework\V4.0.30319\msbuild.exe /maxcpucount /p:Configuration=Release /t:Clean
c:\Windows\Microsoft.Net\Framework\V4.0.30319\msbuild.exe /maxcpucount /p:Configuration=Release

copy TrenchBroom\FreeImage.dll Release\
copy TrenchBroom\freetype6.dll Release\
copy TrenchBroom\ftgl.dll      Release\
copy TrenchBroom\zlib1.dll     Release\

del Release\TrenchBroom.pdb

cd Release

For /f "tokens=1,2,3,4,5 delims=/. " %%a in ('date/T') do set CDate=%%c%%b%%a
For /f "tokens=1,2 delims=:" %%f in ('time /t') do set CTime=%%f%%g

C:\Programme\7-Zip\7z.exe a "C:\Users\kristian\Dropbox\TrenchBroom\TrenchBroom_Win32_%CDATE%_%CTIME%.zip" .\*

pause