# Download MediaMTX (aler9/mediamtx) binary for Windows x64 and place into src-tauri/mediamtx/
$dest = Join-Path -Path $PSScriptRoot -ChildPath "..\src-tauri\mediamtx"
if (!(Test-Path $dest)) { New-Item -ItemType Directory -Path $dest | Out-Null }
$filename = "mediamtx.exe"
$full = Join-Path $dest $filename
Write-Output "Will download mediamtx to: $full"

# You may need to update the URL to the correct release for your platform
$releaseUrl = "https://github.com/aler9/mediamtx/releases/latest/download/mediamtx_windows_amd64.exe"

try {
    Invoke-WebRequest -Uri $releaseUrl -OutFile $full -UseBasicParsing -ErrorAction Stop
    Write-Output "Downloaded mediamtx to $full"
} catch {
    Write-Error "Failed to download mediamtx: $_"
}
