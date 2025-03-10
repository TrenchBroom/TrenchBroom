@echo off
setlocal enabledelayedexpansion

:: Check if PID is provided
if "%~1"=="" (
    echo Error: Please provide the PID of the target process.
    echo Usage: %~nx0 ^<pid^> "C:\path\to\target" "C:\path\to\replacement" ["C:\path\to\app"]
    exit /b 1
)

:: Check if target path is provided
if "%~2"=="" (
    echo Error: Please provide the target folder.
    echo Usage: %~nx0 ^<pid^> "C:\path\to\target" "C:\path\to\replacement" ["C:\path\to\app"]
    exit /b 1
)
if "%~3"=="" (
    echo Error: Please provide the replacement folder.
    echo Usage: %~nx0 ^<pid^> "C:\path\to\target" "C:\path\to\replacement" ["C:\path\to\app"]
    exit /b 1
)

set "PID=%~1"
set "TIMEOUT=30"
set "ELAPSED=0"

set "TARGET=%~2"
set "REPLACEMENT=%~3"

if "%~4" neq "" (
  set "APP=%~4"
) else (
  set "APP="
)

:: Check if the replacement folder exists
if not exist "%REPLACEMENT%" (
    echo Error: Source folder "%REPLACEMENT%" does not exist.
    exit /b 1
)

:: Check if the target folder exists
if not exist "%TARGET%" (
    echo Error: Destination folder "%TARGET%" does not exist.
    exit /b 1
)

:CHECK_PROCESS
:: Check if the process exists using tasklist
for /f "tokens=1" %%A in ('tasklist /FI "PID eq %PID%" ^| findstr /R "^[0-9]"') do (
    if %ELAPSED% GEQ %TIMEOUT% (
        echo Timeout reached. Exiting.
        exit /b 1
    )
    timeout /t 1 >nul
    set /a ELAPSED+=1
    goto CHECK_PROCESS
)

echo Process with PID %PID% has terminated.

:: Delete all files and subfolders in the target folder
echo Deleting files and subfolders in "%TARGET%"...
rd /s /q "%TARGET%"

:: Copy the contents of the replacement folder to the target
echo Replacing the contents of "%TARGET%" with "%REPLACEMENT%"...
xcopy "%REPLACEMENT%\*" "%TARGET%\" /e /h /y

echo Deleting files and subfolders in "%REPLACEMENT%"...
rd /s /q "%REPLACEMENT%"

echo Operation completed successfully.

if "%APP%" neq "" (
  :: Check if the app exists
  if not exist "%APP%" (
      echo Error: Application "%APP%" does not exist.
      exit /b 1
  )

  :: Start the app
  echo Starting the application...
  start "" "%APP%"
)
exit /b 0
