@echo off
setlocal

set EXE=%~dp0Release\ShaderBuilder.exe
if not exist "%EXE%" set EXE=%~dp0Release\ShaderBuilder.exe
if not exist "%EXE%" (
    echo ShaderBuilder.exe not found. Build ShaderBuilder.sln first.
    pause
    exit /b 1
)

set IN=%~dp0Shaders\outline.vsh
set OUT=%~dp0Client\Shader\outline.vso

if not exist "%~dp0Client\Shader" mkdir "%~dp0Client\Shader"

echo Input : "%IN%"
echo Output: "%OUT%"

"%EXE%" "%IN%" "%OUT%"
echo ExitCode=%errorlevel%

if exist "%OUT%" (
    echo OK: "%OUT%"
    dir "%OUT%"
) else (
    echo FAIL: output file was not created.
)

pause
