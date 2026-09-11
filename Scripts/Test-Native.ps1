param([string]$Compiler = 'g++', [int]$TimeoutSeconds = 60)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot ('Saved/Tests/Native/' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $output -Force | Out-Null
$results = @()
foreach ($source in Get-ChildItem (Join-Path $projectRoot 'Tests/*Tests.cpp') | Sort-Object Name) {
    $exe = Join-Path $output ($source.BaseName + '.exe')
    $buildLog = Join-Path $output ($source.BaseName + '.build.log')
    & $Compiler '-std=c++17' '-O2' '-Wall' '-Wextra' $source.FullName '-o' $exe *> $buildLog
    if ($LASTEXITCODE -ne 0) {
        $results += [pscustomobject]@{name=$source.BaseName; status='compile-failed'; log=$buildLog}
        continue
    }
    $log = Join-Path $output ($source.BaseName + '.log')
    $errorLog = Join-Path $output ($source.BaseName + '.stderr.log')
    $launchOptions = @{
        FilePath=$exe; PassThru=$true
        RedirectStandardOutput=$log; RedirectStandardError=$errorLog
    }
    if ($IsWindows) { $launchOptions.WindowStyle = 'Hidden' }
    $process = Start-Process @launchOptions
    if (!$process.WaitForExit($TimeoutSeconds * 1000)) {
        $process.Kill(); $process.WaitForExit()
        $status = 'timeout'
    } else {
        $process.WaitForExit()
        $status = if ($process.ExitCode -eq 0) { 'passed' } else { 'failed' }
    }
    $results += [pscustomobject]@{name=$source.BaseName; status=$status; log=$log; stderr=$errorLog}
}
$results | ConvertTo-Json -Depth 4 | Set-Content (Join-Path $output 'results.json') -Encoding utf8
$results | Format-Table name,status
Write-Host "Rapport : $output"
if ($results.Count -eq 0 -or @($results | Where-Object status -ne 'passed').Count -gt 0) { exit 1 }
