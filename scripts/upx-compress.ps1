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
            & upx -9 --best --no-progress $file
            $exitCode = $LASTEXITCODE
            if ($exitCode -ne 0) {
                Write-Warning "UPX returned code $exitCode on $file, skipping"
                $LASTEXITCODE = 0
            }
        } catch {
            Write-Warning "UPX failed on $file: $($_.Exception.Message)"
            $LASTEXITCODE = 0
        }
    }
}

$LASTEXITCODE = 0

Write-Host "UPX compression finished."
