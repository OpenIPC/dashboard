@echo off
echo Installing Tesseract OCR for Windows...

REM Download Tesseract installer
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/UB-Mannheim/tesseract/wiki/Downloading-Tesseract-OCR' -OutFile 'tesseract_info.txt'}"

echo.
echo Please download and install Tesseract OCR from:
echo https://github.com/UB-Mannheim/tesseract/wiki
echo.
echo Or use Chocolatey:
echo choco install tesseract
echo.
echo After installation, add Tesseract to PATH or set TESSDATA_PREFIX environment variable.

pause