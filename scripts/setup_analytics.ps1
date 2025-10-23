<#
Simple PowerShell helper to create a venv for analytics and install ONNX Runtime DirectML.
Run from project root in PowerShell (as a user):
  .\scripts\setup_analytics.ps1

This will create .analytics_venvs\dml\ and install packages from python_src\requirements_analytics.txt.
Note: DirectML requires Windows 10/11 with appropriate GPU drivers (DirectX 12) and "onnxruntime-directml" support.
#>

$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $root
if (-not (Test-Path -Path '.analytics_venvs')) { New-Item -ItemType Directory -Path '.analytics_venvs' | Out-Null }
$venvPath = Join-Path -Path '.analytics_venvs' -ChildPath 'dml'

if (-not (Test-Path -Path $venvPath)) {
    python -m venv $venvPath
}

$pip = Join-Path -Path $venvPath -ChildPath 'Scripts\pip.exe'
$requirements = Join-Path -Path $root -ChildPath 'python_src\requirements_analytics.txt'
if (-not (Test-Path -Path $pip)) { Write-Error "pip not found in $venvPath\Scripts - ensure python venv creation succeeded"; exit 1 }

& $pip install --upgrade pip setuptools wheel
& $pip install -r $requirements

Write-Output "Analytics venv ready at: $venvPath"
Write-Output "To use it, set your PYTHON executable path in the app or ensure .analytics_venvs\dml\Scripts\python.exe is available"
