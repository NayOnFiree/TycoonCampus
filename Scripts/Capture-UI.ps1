param(
    [string]$EngineRoot = 'D:/Epic Games/UE_5.8',
    [string]$BuildProject,
    [switch]$SkipBuild,
    [ValidateSet('all', '1280x720', '1920x1080')][string]$Resolution = 'all',
    [ValidateSet('1', '1.25', '1.5')][string[]]$Scales = @('1', '1.25', '1.5'),
    [ValidateRange(30, 1200)][int]$TimeoutSeconds = 240
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$testsRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot 'Saved/Tests'))
$output = Join-Path $testsRoot ('UIVisual/' + [guid]::NewGuid().ToString('N'))
$editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$build = Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat'
if (!(Test-Path -LiteralPath $editor)) { throw "Moteur introuvable : $editor" }
if (!$BuildProject) { $BuildProject = Join-Path $output 'Project' }
$BuildProject = [IO.Path]::GetFullPath($BuildProject)
if (!$BuildProject.StartsWith($testsRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'BuildProject must be a disposable project below this worktree Saved/Tests.'
}
# Refuse path redirection before copying or writing any fixture files.
$ancestor = $BuildProject
while ($ancestor -and $ancestor.Length -ge $testsRoot.Length) {
    if ((Test-Path -LiteralPath $ancestor) -and ((Get-Item -LiteralPath $ancestor).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw "Reparse points are not allowed in the isolated project path: $ancestor"
    }
    $ancestor = Split-Path $ancestor -Parent
}
foreach ($folder in @('Source', 'Config', 'Content')) {
    $target = Join-Path $BuildProject $folder
    if (Test-Path -LiteralPath $target) {
        if ((Get-Item -LiteralPath $target).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected fixture folder refused.' }
        if (@(Get-ChildItem -LiteralPath $target -Recurse -Force | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) {
            throw 'Redirected fixture descendant refused.'
        }
    }
}
foreach ($asset in Get-ChildItem (Join-Path $projectRoot 'Content') -Recurse -File) {
    if ($asset.Length -lt 1024 -and (Get-Content -LiteralPath $asset.FullName -TotalCount 1) -eq 'version https://git-lfs.github.com/spec/v1') {
        throw "Asset LFS non hydrate : $($asset.FullName). Executer git lfs pull avant le test."
    }
}
$sourceFiles = @(Get-ChildItem (Join-Path $projectRoot 'Source') -Recurse -File)
$sourceFiles += @(Get-ChildItem (Join-Path $projectRoot 'Config') -Recurse -File)
$sourceFiles += Get-Item (Join-Path $projectRoot 'TycoonCampus.uproject')
$sourceDigest = ($sourceFiles | Sort-Object FullName | ForEach-Object {
    [IO.Path]::GetRelativePath($projectRoot, $_.FullName) + ':' + (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
}) -join "`n"
$stamp = Join-Path $BuildProject 'CampusVisualBuild.txt'
if ($SkipBuild) {
    if (!(Test-Path -LiteralPath $stamp) -or (Get-Content -Raw -LiteralPath $stamp).TrimEnd() -ne $sourceDigest.TrimEnd()) {
        throw 'SkipBuild requires a successful capture-harness build with identical Source/Config/project files.'
    }
    foreach ($file in $sourceFiles) {
        $relative = [IO.Path]::GetRelativePath($projectRoot, $file.FullName)
        $target = Join-Path $BuildProject $relative
        if (!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target).Hash -ne (Get-FileHash -LiteralPath $file.FullName).Hash) {
            throw "SkipBuild cache source differs from the checkout: $relative"
        }
    }
} else {
    New-Item -ItemType Directory -Force -Path $BuildProject | Out-Null
    # A reused cache may not silently retain source files deleted from the checkout.
    $sourceTarget = Join-Path $BuildProject 'Source'
    if (Test-Path -LiteralPath $sourceTarget) {
        foreach ($file in Get-ChildItem -LiteralPath $sourceTarget -Recurse -File) {
            $relative = [IO.Path]::GetRelativePath($sourceTarget, $file.FullName)
            if (!(Test-Path -LiteralPath (Join-Path (Join-Path $projectRoot 'Source') $relative))) {
                throw "Stale source in build cache; use a fresh BuildProject: $relative"
            }
        }
    }
    $changedSources = @(Get-ChildItem (Join-Path $projectRoot 'Source') -Recurse -File | Where-Object {
        $target = Join-Path $BuildProject ([IO.Path]::GetRelativePath($projectRoot, $_.FullName))
        !(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target).Hash -ne (Get-FileHash -LiteralPath $_.FullName).Hash
    } | ForEach-Object { Join-Path $BuildProject ([IO.Path]::GetRelativePath($projectRoot, $_.FullName)) })
    foreach ($folder in @('Source', 'Config', 'Content')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot $folder) -Destination $BuildProject -Recurse -Force
    }
    Copy-Item -LiteralPath (Join-Path $projectRoot 'TycoonCampus.uproject') -Destination $BuildProject -Force
    # Copy-Item preserves source timestamps; a reused object may be newer than changed source.
    foreach ($file in $changedSources) { (Get-Item -LiteralPath $file).LastWriteTime = Get-Date }
}
New-Item -ItemType Directory -Force -Path $output | Out-Null
Set-Content -LiteralPath (Join-Path $BuildProject 'CampusVisualFixture.txt') -Value 'isolated-ui-visual-v1' -Encoding utf8
$project = Join-Path $BuildProject 'TycoonCampus.uproject'
$commit = (& git -C $projectRoot rev-parse HEAD 2>$null) -join ''
$dirty = @(& git -C $projectRoot status --porcelain --untracked-files=normal 2>$null).Count -gt 0
$harnessHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash
if (!$SkipBuild) {
    & $build TycoonCampusEditor Win64 Development "-Project=$project" '-WaitMutex' '-NoHotReloadFromIDE' '-NoUBA' '-MaxParallelActions=2' *> (Join-Path $output 'build.log')
    if ($LASTEXITCODE -ne 0) { throw "Compilation en echec : $output/build.log" }
    Set-Content -LiteralPath $stamp -Value $sourceDigest -Encoding utf8
}
$resolutions = if ($Resolution -eq 'all') { @('1280x720', '1920x1080') } else { @($Resolution) }
$results = @()
foreach ($size in $resolutions) {
    $dimensions = $size -split 'x'
    foreach ($scale in $Scales) {
        $percent = [int]([double]::Parse($scale, [Globalization.CultureInfo]::InvariantCulture) * 100)
        $case = Join-Path $output ($size + '_ui' + $percent)
        $caseProfile = Join-Path $case 'Profile'
        New-Item -ItemType Directory -Force -Path $caseProfile | Out-Null
        $arguments = @(
            "`"$project`"", '/Game/TycoonCampus/Maps/L_Campus_Work', '-game', '-unattended', '-nosound',
            '-windowed', '-RenderOffscreen', '-ForceRes', '-NoSplash', '-CampusVisualAutomation',
            "-ResX=$($dimensions[0])", "-ResY=$($dimensions[1])", "-CampusVisualScale=$scale",
            "-UserDir=`"$caseProfile`"", "-CampusCaptureDir=`"$case`"",
            '-ExecCmds="Automation RunTests TycoonCampus.Visual.CaptureUI"', '-TestExit="Automation Test Queue Empty"',
            "-ReportExportPath=`"$case`"", "-abslog=`"$case/runtime.log`""
        )
        $started = [DateTime]::UtcNow.ToString('o')
        $process = Start-Process -FilePath $editor -ArgumentList $arguments -WindowStyle Hidden -PassThru
        $status = 'failed'
        $warnings = @()
        $renderDevice = $null
        if (!$process.WaitForExit($TimeoutSeconds * 1000)) {
            $process.Kill(); $process.WaitForExit(); $status = 'timeout'
        } else {
            $process.WaitForExit()
            $reportPath = Join-Path $case 'index.json'
            if ($process.ExitCode -eq 0 -and (Test-Path -LiteralPath $reportPath)) {
                $report = Get-Content -Raw -LiteralPath $reportPath | ConvertFrom-Json
                $captures = @(Get-ChildItem -LiteralPath $case -Filter '*.png' -File)
                $warnings = @($report.tests | ForEach-Object { $_.entries } | Where-Object { $_.event.type -eq 'Warning' } | ForEach-Object { $_.event.message })
                $renderDevice = $report.devices | Select-Object -First 1 rhi,renderMode,gpu
                if (($report.succeeded + $report.succeededWithWarnings) -eq 1 -and $report.failed -eq 0 -and $report.notRun -eq 0 -and $report.inProcess -eq 0 -and $captures.Count -eq 2) {
                    $status = if ($report.succeededWithWarnings -gt 0) { 'passed-with-warnings' } else { 'passed' }
                }
            }
        }
        $results += [pscustomobject]@{resolution=$size; requestedEffectiveUIScale=$scale; status=$status; warnings=$warnings; renderDevice=$renderDevice; output=$case; startedUTC=$started; exitCode=$process.ExitCode}
        [pscustomobject]@{schemaVersion=1; commit=$commit; dirty=$dirty; sourceHashes=$sourceDigest; harnessSHA256=$harnessHash; engineRoot=$EngineRoot;
            captureMode='real RHI RenderOffscreen with Slate UI'; windowsDPIChanged=$false; results=$results} |
            ConvertTo-Json -Depth 5 | Set-Content (Join-Path $output 'matrix.json') -Encoding utf8
        Write-Host "$size UI $scale : $status"
    }
}
Write-Host "Matrice : $output/matrix.json"
if (@($results | Where-Object { $_.status -notin @('passed', 'passed-with-warnings') }).Count) { exit 1 }
