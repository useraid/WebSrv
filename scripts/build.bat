@echo off
setlocal

REM Dependency Checks
where cmake >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo "CMake Not Found"
    exit /b 1
)

REM Project Directory
cd ..

REM Create Build Folder
if not exist build (
    mkdir build
)

REM Change to working directory
cd build

REM Generate Makefile
cmake ..
if %ERRORLEVEL% NEQ 0 (
    echo "Makefile failed."
    exit /b 1
)

REM Build Server
cmake --build .
if %ERRORLEVEL% NEQ 0 (
    echo "Build failed."
    exit /b 1
)

echo "Build successfully, run it with server.exe"

endlocal
