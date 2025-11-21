# ONNX Runtime DirectML Installer
# Downloads and installs ONNX Runtime 1.20+ with DirectML support

$ErrorActionPreference = "Stop"

Write-Host "=== ONNX Runtime DirectML Installer ===" -ForegroundColor Cyan
Write-Host ""

# Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    exit 1
}

# ONNX Runtime version to download
$version = "1.20.1"
$downloadUrl = "https://github.com/microsoft/onnxruntime/releases/download/v$version/onnxruntime-win-x64-gpu-$version.zip"
$tempDir = "$env:TEMP\onnxruntime-install"
$zipFile = "$tempDir\onnxruntime.zip"
$extractDir = "$tempDir\extracted"

Write-Host "1. Checking current ONNX Runtime version..." -ForegroundColor Yellow
$currentDll = "C:\Windows\System32\onnxruntime.dll"
if (Test-Path $currentDll) {
    $fileInfo = Get-Item $currentDll
    Write-Host "   Found: $currentDll" -ForegroundColor Gray
    Write-Host "   Size: $($fileInfo.Length) bytes" -ForegroundColor Gray
    Write-Host "   Modified: $($fileInfo.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "   No existing onnxruntime.dll found" -ForegroundColor Gray
}

Write-Host ""
Write-Host "2. Creating temporary directory..." -ForegroundColor Yellow
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $tempDir -Force | Out-Null
New-Item -ItemType Directory -Path $extractDir -Force | Out-Null

Write-Host ""
Write-Host "3. Downloading ONNX Runtime $version with DirectML..." -ForegroundColor Yellow
Write-Host "   URL: $downloadUrl" -ForegroundColor Gray
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $zipFile -UseBasicParsing
    Write-Host "   ✓ Download complete" -ForegroundColor Green
} catch {
    Write-Host "   ✗ Download failed: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "4. Extracting archive..." -ForegroundColor Yellow
try {
    Expand-Archive -Path $zipFile -DestinationPath $extractDir -Force
    Write-Host "   ✓ Extraction complete" -ForegroundColor Green
} catch {
    Write-Host "   ✗ Extraction failed: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "5. Locating DLL files..." -ForegroundColor Yellow
$dllFiles = Get-ChildItem -Path $extractDir -Filter "onnxruntime*.dll" -Recurse
if ($dllFiles.Count -eq 0) {
    Write-Host "   ✗ No DLL files found in archive" -ForegroundColor Red
    exit 1
}

foreach ($dll in $dllFiles) {
    Write-Host "   Found: $($dll.Name) ($($dll.Length) bytes)" -ForegroundColor Gray
}

Write-Host ""
Write-Host "6. Backing up existing DLL..." -ForegroundColor Yellow
if (Test-Path $currentDll) {
    $backupPath = "$currentDll.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
    Copy-Item $currentDll $backupPath -Force
    Write-Host "   ✓ Backup created: $backupPath" -ForegroundColor Green
}

Write-Host ""
Write-Host "7. Installing new ONNX Runtime DLL..." -ForegroundColor Yellow
$mainDll = $dllFiles | Where-Object { $_.Name -eq "onnxruntime.dll" } | Select-Object -First 1
if ($mainDll) {
    try {
        Copy-Item $mainDll.FullName $currentDll -Force
        Write-Host "   ✓ DLL installed successfully" -ForegroundColor Green
    } catch {
        Write-Host "   ✗ Failed to copy DLL: $_" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "   ✗ onnxruntime.dll not found in archive" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "8. Looking for DirectML.dll..." -ForegroundColor Yellow
$directmlDll = Get-ChildItem -Path $extractDir -Filter "DirectML.dll" -Recurse | Select-Object -First 1
if ($directmlDll) {
    try {
        Copy-Item $directmlDll.FullName "C:\Windows\System32\DirectML.dll" -Force
        Write-Host "   ✓ DirectML.dll installed" -ForegroundColor Green
    } catch {
        Write-Host "   ⚠ Could not install DirectML.dll: $_" -ForegroundColor Yellow
    }
} else {
    Write-Host "   ℹ DirectML.dll not found (may be in separate package)" -ForegroundColor Gray
}

Write-Host ""
Write-Host "9. Cleaning up..." -ForegroundColor Yellow
Remove-Item $tempDir -Recurse -Force
Write-Host "   ✓ Temporary files removed" -ForegroundColor Green

Write-Host ""
Write-Host "=== Installation Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Restart your application" -ForegroundColor White
Write-Host "  2. Check logs for 'DirectML GPU acceleration is ENABLED'" -ForegroundColor White
Write-Host "  3. If still not working, update GPU drivers" -ForegroundColor White
Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
