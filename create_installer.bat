@echo off
setlocal

set QT_BIN_DIR=C:\OpenIPC-Dashboard-Cpp\6.4.2\mingw_64\bin
set PATH=%QT_BIN_DIR%;%PATH%

echo Cleaning up previous builds...
if exist build_release rmdir /s /q build_release
if exist dist rmdir /s /q dist

echo Configuring Release build...
mkdir build_release
cd build_release
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DQt6_DIR="C:\OpenIPC-Dashboard-Cpp\6.4.2\mingw_64\lib\cmake\Qt6" -DCMAKE_PREFIX_PATH="%QT_BIN_DIR%\.." -DCMAKE_PROGRAM_PATH="%QT_BIN_DIR%" -DQT_RCC_EXECUTABLE="%QT_BIN_DIR%\rcc.exe" -DQt6Core_RCC_EXECUTABLE="%QT_BIN_DIR%\rcc.exe" ..
if %errorlevel% neq 0 (
    echo CMake configuration failed
    exit /b %errorlevel%
)

echo Building...
cmake --build .
if %errorlevel% neq 0 (
    echo Build failed
    exit /b %errorlevel%
)

echo Creating distribution folder...
cd ..
mkdir dist
copy build_release\appOpenIPC-Dashboard.exe dist\
xcopy /E /I /Y build_release\OpenIPC dist\OpenIPC

echo Deploying Qt dependencies...
"%QT_BIN_DIR%\windeployqt.exe" --qmldir src\ui --compiler-runtime dist\appOpenIPC-Dashboard.exe
if %errorlevel% neq 0 (
    echo windeployqt failed
    exit /b %errorlevel%
)

echo Copying missing Qt DLLs...
if exist "%QT_BIN_DIR%\Qt6ShaderTools.dll" copy "%QT_BIN_DIR%\Qt6ShaderTools.dll" dist\
if exist "%QT_BIN_DIR%\Qt6Svg.dll" copy "%QT_BIN_DIR%\Qt6Svg.dll" dist\

echo Copying MDK dependencies...
copy mdk-sdk\bin\x64\*.dll dist\

echo Copying ONNX Runtime dependencies...
copy libs\onnxruntime\lib\*.dll dist\

echo Copying Dahua SDK dependencies...
copy libs\dahua\bin\*.dll dist\

echo Creating qt.conf...
(
echo [Paths]
echo Prefix=.
echo Plugins=.
echo Imports=.
echo Qml2Imports=.
) > dist\qt.conf

echo Copying MSVC runtime (if needed by MDK)...
if exist C:\Windows\System32\vcruntime140.dll copy C:\Windows\System32\vcruntime140.dll dist\
if exist C:\Windows\System32\msvcp140.dll copy C:\Windows\System32\msvcp140.dll dist\

echo.
echo ========================================================
echo  Build complete!
echo  You can find the portable application in the 'dist' folder.
echo  Zip this folder to share it with others.
echo ========================================================
pause
