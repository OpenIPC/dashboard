@echo off
setlocal

set "QT_PATH=%~dp06.4.2\mingw_64"

if not exist "%QT_PATH%" (
    echo ERROR: Qt path not found: %QT_PATH%
    echo Please edit this file configure.bat and specify the correct Qt path.
    pause
    exit /b 1
)

if not exist "build" mkdir "build"
cd "build"

cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="%QT_PATH%" ..
if errorlevel 1 (
    echo CMake configuration error.
    pause
    exit /b 1
)

echo.
echo Configuration successful! You can now build the project with:
echo cmake --build . --config Release
echo.
pause
endlocal
