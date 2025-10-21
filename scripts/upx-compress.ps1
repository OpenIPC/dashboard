param(
    [string]$DistPath = "dist"
)

Write-Host "Compressing artifacts in $DistPath with UPX..."
if (-not (Test-Path $DistPath)) {
    Write-Error "Dist directory not found: $DistPath"
    exit 1
}

Get-ChildItem -Path $DistPath -Recurse -File | ForEach-Object {
    $file = $_.FullName
    if ($file -like "*.exe" -or $file -like "*.AppImage") {
        Write-Host "Processing: $file"
        try {
            & upx -9 --best $file
        } catch {
            Write-Warning "UPX failed on $file"
        }
    }
}

Write-Host "UPX compression finished."
