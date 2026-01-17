@echo off
setlocal

:: Set paths to dependencies
set "GST_BIN=C:\Program Files\gstreamer\1.0\mingw_x86_64\bin"
set "GST_PLUGIN_PATH=C:\Program Files\gstreamer\1.0\mingw_x86_64\lib\gstreamer-1.0"
set "QT_BIN=%~dp06.4.2\mingw_64\bin"
set "DAHUA_BIN=%~dp0libs\dahua\bin"
set "ONNX_BIN=%~dp0libs\onnxruntime\lib"

:: Add to PATH
set "PATH=%QT_BIN%;%GST_BIN%;%DAHUA_BIN%;%ONNX_BIN%;%PATH%"

echo Starting OpenIPC Dashboard...
echo GStreamer Path: %GST_BIN%

"build\appOpenIPC-Dashboard.exe"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Application exited with error code %ERRORLEVEL%
)

pause

endlocal
