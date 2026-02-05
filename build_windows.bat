@echo off
setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [ERROR] vswhere.exe not found.
    exit /b 1
)

set "VS_PATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
    echo [ERROR] Visual Studio not found.
    exit /b 1
)

if exist "%VS_PATH%\Common7\Tools\VsDevCmd.bat" (
    call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=x64 -no_logo
) else (
    echo [ERROR] VsDevCmd.bat not found.
    exit /b 1
)

if exist build (
    rmdir /s /q build
)
mkdir build

cmake -S . -B build -G "NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE="vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Debug
if %errorlevel% neq 0 (
    echo [ERROR] Configuration failed.
    exit /b 1
)

cmake --build build --config Debug
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo Build Success.
endlocal
