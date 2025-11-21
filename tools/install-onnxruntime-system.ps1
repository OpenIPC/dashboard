# Install ONNX Runtime DirectML to system directory
# Requires Administrator privileges

#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "=== ONNX Runtime DirectML System Installer ===" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = Split-Path $PSScriptRoot -Parent
$BinariesDir = Join-Path $ProjectRoot "src-tauri\binaries"
$SystemDir = "C:\Windows\System32"

# Check if source DLL exists
$SourceDLL = Join-Path $BinariesDir "onnxruntime.dll"
if (-not (Test-Path $SourceDLL)) {
    Write-Host "Error: Source DLL not found at: $SourceDLL" -ForegroundColor Red
    Write-Host "Run 'python tools\download-directml-nuget.py' first!" -ForegroundColor Yellow
    exit 1
}

# Get version info
$SourceVersion = (Get-Item $SourceDLL).VersionInfo.FileVersion
$SourceSize = [math]::Round((Get-Item $SourceDLL).Length / 1MB, 1)

Write-Host "Source DLL:" -ForegroundColor Green
Write-Host "  Path: $SourceDLL"
Write-Host "  Size: $SourceSize MB"
Write-Host "  Version: $SourceVersion"
Write-Host ""

# Check existing system DLL
$SystemDLL = Join-Path $SystemDir "onnxruntime.dll"
$BackupDLL = Join-Path $SystemDir "onnxruntime.dll.backup"

if (Test-Path $SystemDLL) {
    $SystemVersion = (Get-Item $SystemDLL).VersionInfo.FileVersion
    $SystemSize = [math]::Round((Get-Item $SystemDLL).Length / 1MB, 1)
    
    Write-Host "Existing system DLL:" -ForegroundColor Yellow
    Write-Host "  Path: $SystemDLL"
    Write-Host "  Size: $SystemSize MB"
    Write-Host "  Version: $SystemVersion"
    Write-Host ""
    
    # Create backup
    Write-Host "Creating backup..." -ForegroundColor Cyan
    if (Test-Path $BackupDLL) {
        Write-Host "  Backup already exists: $BackupDLL" -ForegroundColor Yellow
    } else {
        Copy-Item $SystemDLL $BackupDLL -Force
        Write-Host "  ✓ Backup created: $BackupDLL" -ForegroundColor Green
    }
    Write-Host ""
}

# Ask for confirmation
Write-Host "This will replace the system ONNX Runtime DLL." -ForegroundColor Yellow
Write-Host "This may affect other applications using ONNX Runtime." -ForegroundColor Yellow
Write-Host ""
$Confirm = Read-Host "Continue? (yes/no)"

if ($Confirm -ne "yes" -and $Confirm -ne "y") {
    Write-Host "Installation cancelled." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "Installing..." -ForegroundColor Cyan

try {
    # Copy main DLL
    Copy-Item $SourceDLL $SystemDLL -Force
    Write-Host "  ✓ onnxruntime.dll installed" -ForegroundColor Green
    
    # Copy shared provider DLL if exists
    $SharedProvider = Join-Path $BinariesDir "onnxruntime_providers_shared.dll"
    if (Test-Path $SharedProvider) {
        $DestProvider = Join-Path $SystemDir "onnxruntime_providers_shared.dll"
        Copy-Item $SharedProvider $DestProvider -Force
        Write-Host "  ✓ onnxruntime_providers_shared.dll installed" -ForegroundColor Green
    }
    
    Write-Host ""
    Write-Host "=== Installation Complete ===" -ForegroundColor Green
    Write-Host ""
    Write-Host "Installed files:" -ForegroundColor Cyan
    Get-ChildItem $SystemDir -Filter "onnxruntime*.dll" | ForEach-Object {
        $Size = [math]::Round($_.Length / 1MB, 1)
        Write-Host "  - $($_.Name) ($Size MB)"
    }
    
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. Test DirectML: cargo run --manifest-path src-tauri/Cargo.toml --example check_directml"
    Write-Host "  2. Run app: npm run tauri dev"
    Write-Host "  3. Check for '✓ DirectML GPU acceleration is ENABLED' in logs"
    Write-Host ""
    Write-Host "To rollback:" -ForegroundColor Yellow
    Write-Host "  Copy-Item '$BackupDLL' '$SystemDLL' -Force"
    Write-Host ""
    
} catch {
    Write-Host ""
    Write-Host "Installation failed: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "Make sure to run PowerShell as Administrator!" -ForegroundColor Yellow
    exit 1
}
