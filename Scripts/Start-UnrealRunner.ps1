# Starts the repository-scoped runner already registered on this PC; no credentials in Git.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$runnerRoot = Join-Path $projectRoot '.ci-runner'
$runFile = Join-Path $runnerRoot 'run.cmd'
if (!(Test-Path (Join-Path $runnerRoot '.runner'))) {
    throw 'Runner non configure sur cette machine. Voir Docs/Collaboration.md et les reglages Actions du depot.'
}
$active = Get-CimInstance Win32_Process -Filter "Name = 'Runner.Listener.exe'" |
    Where-Object { $_.ExecutablePath -and $_.ExecutablePath.StartsWith($runnerRoot, [StringComparison]::OrdinalIgnoreCase) }
if ($active) { Write-Host 'Runner Unreal deja actif.'; exit 0 }
$process = Start-Process -FilePath $runFile -WorkingDirectory $runnerRoot -WindowStyle Hidden -PassThru
Write-Host "Runner Unreal lance (processus $($process.Id)). Etat dans GitHub > Settings > Actions > Runners."
