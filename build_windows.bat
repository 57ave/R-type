@echo off
setlocal

:: Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 goto cmake_found

echo [INFO] CMake not found.
where winget >nul 2>nul
if %ERRORLEVEL% NEQ 0 goto no_cmake_no_winget

echo [INFO] Attempting to install CMake via winget...
winget install -e --id Kitware.CMake --accept-source-agreements --accept-package-agreements
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] CMake installed successfully.
    echo [INFO] Please restart your terminal to update the PATH, then run this script again.
    exit /b 0
) else (
    echo [ERROR] Failed to install CMake via winget.
    exit /b 1
)

:no_cmake_no_winget
echo [ERROR] CMake is not installed and winget is not available.
echo [INFO] Please install CMake manually from https://cmake.org/download/
exit /b 1

:cmake_found

echo [INFO] Configuring project with CMake...
:: Use the windows-release preset as default
:: Note: Dependencies are managed via CPM (cached in build directory)
cmake --preset windows-release

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

echo [INFO] Building project...
cmake --build --preset windows-release --config Release

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo [SUCCESS] Build completed successfully.
echo Output binaries are in build\windows-release\Release\ (or Debug\)
endlocal
