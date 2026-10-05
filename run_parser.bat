@echo off
setlocal
cd /d "%~dp0"
if not exist "%~2" (
    mkdir "%~2" 2>nul
)
"%~dp0JSBSimXMLParser.exe" --xml %1 --out %2 --prefix %3 --aircraft %4 --parse-aero %5 --parse-fcs %6 --parse-metrics %7
exit /b %ERRORLEVEL%
