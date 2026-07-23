param(
    [Parameter(Mandatory = $true)]
    [string]$Executable,
    [string]$TaskName = "OpenIPC Dashboard Server",
    [string]$DataRoot = (Join-Path $env:LOCALAPPDATA "OpenIPC Dashboard Server")
)

$ErrorActionPreference = "Stop"
$resolved = (Resolve-Path -LiteralPath $Executable).Path
$resolvedDataRoot = [IO.Path]::GetFullPath($DataRoot)
New-Item -ItemType Directory -Path $resolvedDataRoot -Force | Out-Null
$action = New-ScheduledTaskAction -Execute $resolved `
    -Argument ('--server-only --data-root "{0}"' -f $resolvedDataRoot)
$trigger = New-ScheduledTaskTrigger -AtLogOn -User $env:USERNAME
$settings = New-ScheduledTaskSettingsSet -ExecutionTimeLimit (New-TimeSpan -Days 3650) `
    -RestartCount 3 -RestartInterval (New-TimeSpan -Minutes 1) -MultipleInstances IgnoreNew
$principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -LogonType Interactive -RunLevel Limited

Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $trigger `
    -Settings $settings -Principal $principal -Description "OpenIPC Dashboard --server-only" -Force

Write-Host "Registered '$TaskName' for $resolved with isolated data root $resolvedDataRoot. Configure the Web deployment profile before starting it."
