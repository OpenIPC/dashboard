# VMS Dashboard Build Script for Windows
# Downloads MediaMTX binaries and builds the application

param(
    [Parameter(HelpMessage="Target platform: windows, linux, macos")]
    [ValidateSet("windows", "linux", "macos")]
    [string]$Platform,
    
    [Parameter(HelpMessage="Build in debug mode")]
    [switch]$Debug,
    
    [Parameter(HelpMessage="Only download MediaMTX binaries")]
    [switch]$DownloadOnly,
    
    [Parameter(HelpMessage="Show help")]
    [switch]$Help
)

function Show-Help {
    Write-Host "VMS Dashboard Build Script" -ForegroundColor Green
    Write-Host "=========================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  .\tools\build.ps1                    # Build for current platform"
    Write-Host "  .\tools\build.ps1 -Platform windows  # Build for Windows"
    Write-Host "  .\tools\build.ps1 -Platform linux    # Build for Linux"
    Write-Host "  .\tools\build.ps1 -Platform macos    # Build for macOS"
    Write-Host "  .\tools\build.ps1 -Debug             # Build in debug mode"
    Write-Host "  .\tools\build.ps1 -DownloadOnly      # Only download binaries"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  npm run build-release                # Build release"
    Write-Host "  npm run build-debug                  # Build debug"
    Write-Host "  npm run download-mediamtx             # Download MediaMTX"
    Write-Host ""
}

function Test-Python {
    try {
        $pythonVersion = python --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Python found: $pythonVersion" -ForegroundColor Green
            return $true
        }
    }
    catch {
        Write-Host "❌ Python not found. Please install Python 3.6+" -ForegroundColor Red
        Write-Host "   Download from: https://www.python.org/downloads/" -ForegroundColor Yellow
        return $false
    }
    return $false
}

function Test-Node {
    try {
        $nodeVersion = node --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Node.js found: $nodeVersion" -ForegroundColor Green
            return $true
        }
    }
    catch {
        Write-Host "❌ Node.js not found. Please install Node.js 18+" -ForegroundColor Red
        Write-Host "   Download from: https://nodejs.org/" -ForegroundColor Yellow
        return $false
    }
    return $false
}

function Test-Rust {
    try {
        $rustVersion = rustc --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Rust found: $rustVersion" -ForegroundColor Green
            return $true
        }
    }
    catch {
        Write-Host "❌ Rust not found. Please install Rust" -ForegroundColor Red
        Write-Host "   Install from: https://rustup.rs/" -ForegroundColor Yellow
        return $false
    }
    return $false
}

function Main {
    if ($Help) {
        Show-Help
        return
    }

    Write-Host "🚀 VMS Dashboard Build Script" -ForegroundColor Cyan
    Write-Host "=============================" -ForegroundColor Cyan
    Write-Host ""

    # Check prerequisites
    Write-Host "🔍 Checking prerequisites..." -ForegroundColor Yellow
    
    $pythonOk = Test-Python
    $nodeOk = Test-Node
    $rustOk = Test-Rust

    if (-not ($pythonOk -and $nodeOk -and $rustOk)) {
        Write-Host ""
        Write-Host "❌ Missing prerequisites. Please install the required tools." -ForegroundColor Red
        exit 1
    }

    Write-Host ""
    Write-Host "🔧 Starting build process..." -ForegroundColor Yellow

    # Prepare Python command
    $pythonArgs = @("tools\build.py")
    
    if ($Platform) {
        $pythonArgs += "--platform", $Platform
    }
    
    if ($Debug) {
        $pythonArgs += "--debug"
    }
    
    if ($DownloadOnly) {
        $pythonArgs += "--download-only"
    }

    # Run Python build script
    try {
        & python @pythonArgs
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host ""
            Write-Host "🎉 Build completed successfully!" -ForegroundColor Green
            Write-Host ""
            Write-Host "📁 Build outputs can be found in:" -ForegroundColor Yellow
            Write-Host "   src-tauri\target\release\bundle\" -ForegroundColor Gray
        } else {
            Write-Host ""
            Write-Host "❌ Build failed with exit code: $LASTEXITCODE" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    }
    catch {
        Write-Host ""
        Write-Host "❌ Build failed: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
}

# Run main function
Main