param(
    [string]$EngineRoot = 'D:/Epic Games/UE_5.8',
    [switch]$SkipBuild,
    [switch]$Isolated,
    [int]$TimeoutSeconds = 180
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$project = Join-Path $projectRoot 'TycoonCampus.uproject'
$editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$build = Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat'
if (!(Test-Path $editor)) { throw "Moteur introuvable : $editor" }
$output = Join-Path $projectRoot ('Saved/Tests/Unreal/' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $output -Force | Out-Null
if ($Isolated) {
    if ($SkipBuild) { throw '-Isolated requires a fresh build; omit -SkipBuild.' }
    $copyRoot = Join-Path $output 'Project'
    New-Item -ItemType Directory -Path $copyRoot | Out-Null
    foreach ($folder in @('Source', 'Config', 'Content')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot $folder) -Destination $copyRoot -Recurse
    }
    Copy-Item -LiteralPath $project -Destination $copyRoot
    $project = Join-Path $copyRoot 'TycoonCampus.uproject'
}
if (!$SkipBuild) {
    & $build TycoonCampusEditor Win64 Development "-Project=$project" '-WaitMutex' '-NoHotReloadFromIDE' '-NoUBA' '-MaxParallelActions=2' *> (Join-Path $output 'build.log')
    if ($LASTEXITCODE -ne 0) { throw "Compilation en echec : $output/build.log" }
}
$arguments = @(
    "`"$project`"", '/Game/TycoonCampus/Maps/L_Campus_Work', '-game', '-unattended', '-nosound', '-nullrhi', '-CampusAutomation',
    '-ExecCmds="Automation RunTests TycoonCampus.Runtime"', '-TestExit="Automation Test Queue Empty"',
    "-ReportExportPath=`"$output`"", "-abslog=`"$output/runtime.log`""
)
$process = Start-Process -FilePath $editor -ArgumentList $arguments -WindowStyle Hidden -PassThru
if (!$process.WaitForExit($TimeoutSeconds * 1000)) {
    $process.Kill(); $process.WaitForExit()
    throw "Timeout Unreal : $output/runtime.log"
}
$process.WaitForExit()
$reportPath = Join-Path $output 'index.json'
if ($process.ExitCode -ne 0 -or !(Test-Path $reportPath)) { throw "Execution ou rapport absent : $output" }
$report = Get-Content $reportPath -Raw | ConvertFrom-Json
if ($null -eq $report.succeeded -or $report.succeeded -lt 1 -or $report.failed -ne 0 -or $report.notRun -ne 0 -or $report.inProcess -ne 0) {
    throw "Tests incomplets ou en echec : $reportPath"
}
Write-Host "Tests Unreal reussis : $($report.succeeded). Rapport : $reportPath"
