@echo off
setlocal
cd /d "%~dp0"

:: Check if cl.exe is already in PATH (e.g. Developer Command Prompt)
where cl.exe >nul 2>nul
if %errorlevel% equ 0 goto COMPILE

echo Configuring MSVC x64 build environment...

:: Attempt to locate vswhere to find Visual Studio installation
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if defined VS_PATH if exist "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" (
    call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
    goto COMPILE
)

:: Fallback standard path
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto COMPILE
)

echo [Error] Failed to find or initialize MSVC x64 environment.
exit /b 1

:COMPILE
echo Compiling JSBSimXMLParser.exe...
cl.exe /std:c++17 /O2 /W3 /EHsc /utf-8 /I src /Fe:JSBSimXMLParser.exe src\main.cpp

if errorlevel 1 (
    echo [Error] Compilation failed.
    exit /b 1
)

:: Clean up intermediate object file
if exist main.obj del main.obj

echo [Success] JSBSimXMLParser.exe built successfully!
dir JSBSimXMLParser.exe
