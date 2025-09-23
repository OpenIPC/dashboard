@echo off
echo Installing Tesseract OCR via Chocolatey...

REM Check if Chocolatey is installed
choco --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Chocolatey is not installed. Installing Chocolatey first...
    powershell -Command "& {Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))}"
)

echo Installing Tesseract OCR...
choco install tesseract -y

echo.
echo Tesseract OCR installed successfully!
echo You may need to restart your command prompt for PATH changes to take effect.
echo.
echo To verify installation, run: tesseract --version

pause