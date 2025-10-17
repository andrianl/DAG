@echo off
REM Path to premake5.exe
set PREMAKE="C:\Path\premake\premake5.exe"

if not exist %PREMAKE% (
    echo Error: premake5.exe not found at %PREMAKE%
    echo Please update the PREMAKE variable in this script with the correct path.
    pause
    exit /b 1
)
echo Generating Visual Studio 2022 project files...
%PREMAKE% vs2022
if errorlevel 1 (
    echo.
    echo Error: Failed to generate project files.
    pause
    exit /b 1
)
echo.
echo Project files generated successfully!
pause