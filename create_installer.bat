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

echo Ensuring Qt platform plugins exist...
if not exist "dist\platforms" mkdir "dist\platforms"
if not exist "dist\qt_plugins\platforms" mkdir "dist\qt_plugins\platforms"
rem Always refresh platform plugins from the Qt SDK to avoid stale or stripped DLLs.
xcopy /E /I /Y "%QT_BIN_DIR%\..\plugins\platforms" "dist\platforms"
xcopy /E /I /Y "%QT_BIN_DIR%\..\plugins\platforms" "dist\qt_plugins\platforms"

echo Ensuring common Qt plugin folders exist...
if not exist "dist\qt_plugins\imageformats" xcopy /E /I /Y "%QT_BIN_DIR%\..\plugins\imageformats" "dist\qt_plugins\imageformats"
if not exist "dist\qt_plugins\styles" xcopy /E /I /Y "%QT_BIN_DIR%\..\plugins\styles" "dist\qt_plugins\styles"
if not exist "dist\qt_plugins\tls" xcopy /E /I /Y "%QT_BIN_DIR%\..\plugins\tls" "dist\qt_plugins\tls"

echo Copying missing Qt DLLs...
if exist "%QT_BIN_DIR%\Qt6ShaderTools.dll" copy "%QT_BIN_DIR%\Qt6ShaderTools.dll" dist\
if exist "%QT_BIN_DIR%\Qt6Svg.dll" copy "%QT_BIN_DIR%\Qt6Svg.dll" dist\

echo Copying GStreamer dependencies...
set "GST_ROOT=C:\Program Files\gstreamer\1.0\mingw_x86_64"
set "GST_BIN=%GST_ROOT%\bin"
set "GST_PLUGINS=%GST_ROOT%\lib\gstreamer-1.0"

:: Create local plugins directory expected by main.cpp
mkdir dist\lib\gstreamer-1.0

:: Copy plugin scanner
if exist "%GST_ROOT%\libexec\gstreamer-1.0\gst-plugin-scanner.exe" copy "%GST_ROOT%\libexec\gstreamer-1.0\gst-plugin-scanner.exe" dist\lib\gstreamer-1.0\
if exist "%GST_ROOT%\bin\gst-plugin-scanner.exe" copy "%GST_ROOT%\bin\gst-plugin-scanner.exe" dist\lib\gstreamer-1.0\

:: Core GStreamer DLLs
copy "%GST_BIN%\libgstreamer-1.0-0.dll" dist\
copy "%GST_BIN%\libgstapp-1.0-0.dll" dist\
copy "%GST_BIN%\libgstbase-1.0-0.dll" dist\
copy "%GST_BIN%\libgstvideo-1.0-0.dll" dist\
copy "%GST_BIN%\libgstaudio-1.0-0.dll" dist\
copy "%GST_BIN%\libgstpbutils-1.0-0.dll" dist\
copy "%GST_BIN%\libgsttag-1.0-0.dll" dist\
copy "%GST_BIN%\libgstcontroller-1.0-0.dll" dist\
copy "%GST_BIN%\libgstnet-1.0-0.dll" dist\
:: Extended GStreamer Libraries (Required by plugins)
copy "%GST_BIN%\libgstgl-1.0-0.dll" dist\
copy "%GST_BIN%\libgstrtp-1.0-0.dll" dist\
copy "%GST_BIN%\libgstrtsp-1.0-0.dll" dist\
copy "%GST_BIN%\libgstsdp-1.0-0.dll" dist\
copy "%GST_BIN%\libgstcodecparsers-1.0-0.dll" dist\
copy "%GST_BIN%\libgstcodecs-1.0-0.dll" dist\
copy "%GST_BIN%\libgstallocators-1.0-0.dll" dist\
copy "%GST_BIN%\libgstriff-1.0-0.dll" dist\

:: Dependencies (GLib, Intl, etc)
copy "%GST_BIN%\libglib-2.0-0.dll" dist\
copy "%GST_BIN%\libgobject-2.0-0.dll" dist\
copy "%GST_BIN%\libgmodule-2.0-0.dll" dist\
copy "%GST_BIN%\libgio-2.0-0.dll" dist\
copy "%GST_BIN%\libintl-8.dll" dist\
copy "%GST_BIN%\libiconv-2.dll" dist\
copy "%GST_BIN%\libffi-7.dll" dist\
copy "%GST_BIN%\liborc-0.4-0.dll" dist\

:: Extra Dependencies (ZLib, XML, Soup, SSL, PCRE, etc)
copy "%GST_BIN%\libz-1.dll" dist\
copy "%GST_BIN%\libbz2.dll" dist\
copy "%GST_BIN%\libxml2-2.dll" dist\
copy "%GST_BIN%\libsoup-3.0-0.dll" dist\
copy "%GST_BIN%\libnghttp2.dll" dist\
copy "%GST_BIN%\libsqlite3-0.dll" dist\
copy "%GST_BIN%\libpsl-5.dll" dist\
copy "%GST_BIN%\libpcre2-8-0.dll" dist\
copy "%GST_BIN%\libssl-3-x64.dll" dist\
copy "%GST_BIN%\libcrypto-3-x64.dll" dist\

:: FFmpeg/Libav dependencies
copy "%GST_BIN%\libavcodec-61.dll" dist\
copy "%GST_BIN%\libavformat-61.dll" dist\
copy "%GST_BIN%\libavutil-59.dll" dist\
copy "%GST_BIN%\libavfilter-10.dll" dist\
copy "%GST_BIN%\libswresample-5.dll" dist\
copy "%GST_BIN%\libswscale-8.dll" dist\

:: Graphics/Text dependencies (Cairo, Pango, Harfbuzz, etc)
copy "%GST_BIN%\libjpeg-8.dll" dist\
copy "%GST_BIN%\libpng16.dll" dist\
copy "%GST_BIN%\libfreetype-6.dll" dist\
copy "%GST_BIN%\libfontconfig-1.dll" dist\
copy "%GST_BIN%\libharfbuzz-0.dll" dist\
copy "%GST_BIN%\libgraphene-1.0-0.dll" dist\
copy "%GST_BIN%\libpango-1.0-0.dll" dist\
copy "%GST_BIN%\libpangocairo-1.0-0.dll" dist\
copy "%GST_BIN%\libpangowin32-1.0-0.dll" dist\
copy "%GST_BIN%\libcairo-2.dll" dist\
copy "%GST_BIN%\libcairo-gobject-2.dll" dist\

:: Plugins
copy "%GST_PLUGINS%\libgstcoreelements.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstplayback.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgsttypefindfunctions.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstapp.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstaudioconvert.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstaudioresample.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstaudioparsers.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstaacparse.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstfaad.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstalaw.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstmulaw.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstopus.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstopusparse.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvoaacenc.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvolume.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstautodetect.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvideoconvert.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvideoconvertscale.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvideoscale.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvideofilters.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvideofilter.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstvideoparsersbad.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstdeinterlace.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstrtp.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstrtpmanager.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstrtsp.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstudp.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgsttcp.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstisomp4.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstmatroska.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstmpegtsdemux.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstde265.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstopenh264.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstlibav.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstwasapi.dll" dist\lib\gstreamer-1.0\
copy "%GST_PLUGINS%\libgstdirectsound.dll" dist\lib\gstreamer-1.0\

echo Copying ONNX Runtime dependencies...
copy libs\onnxruntime\lib\*.dll dist\

echo Copying Dahua SDK dependencies...
copy libs\dahua\bin\*.dll dist\

echo Creating qt.conf...
(
echo [Paths]
echo Prefix=.
echo Plugins=qt_plugins
echo Imports=qml
echo Qml2Imports=qml
) > dist\qt.conf

echo Copying MSVC runtime (if needed)...
if exist C:\Windows\System32\vcruntime140.dll copy C:\Windows\System32\vcruntime140.dll dist\
if exist C:\Windows\System32\msvcp140.dll copy C:\Windows\System32\msvcp140.dll dist\

echo.
echo ========================================================
echo  Build complete!
echo  You can find the portable application in the 'dist' folder.
echo  Zip this folder to share it with others.
echo ========================================================
pause
