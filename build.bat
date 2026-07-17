@echo off
setlocal

:: Compiler selection: respect CC env, then auto-detect (gcc > cl)
if not "%CC%"=="" goto :build

:: Try GCC
where gcc >nul 2>&1
if not errorlevel 1 (
    set "CC=gcc"
    goto :build
)

:: Try MSVC
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" goto :no_compiler
for /f "usebackq tokens=*" %%i in (`"%vswhere%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "InstallDir=%%i"
if not defined InstallDir goto :no_compiler
call "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
where cl >nul 2>&1 || goto :no_compiler
goto :build

:no_compiler
echo ERROR: No C compiler found. Install GCC (MSYS2) or Visual Studio C++ tools.
exit /b 1

:build
cmake -S "%~dp0." -B "%~dp0build" -G Ninja || exit /b 1
cmake --build "%~dp0build" || exit /b 1

if /i "%1"=="run" "%~dp0build\carrot_tests.exe"
if /i "%1"=="demo" (
    echo === cmdscan_demo ===
    "%~dp0build\cmdscan_demo.exe"
    echo.
    echo === cmdqueue_demo ===
    "%~dp0build\cmdqueue_demo.exe"
    echo.
    echo === rpclog_demo ===
    "%~dp0build\rpclog_demo.exe"
    echo.
    echo === dispatch_demo ===
    "%~dp0build\dispatch_demo.exe"
    echo.
    echo === invoke_demo ===
    "%~dp0build\invoke_demo.exe"
    echo.
    echo === ringbuf_demo ===
    "%~dp0build\ringbuf_demo.exe"
)
