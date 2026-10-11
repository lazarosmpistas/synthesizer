[CmdletBinding()]
param(
    [string]$PluginPath,
    [switch]$SkipGuiTests
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'PluginCommon.ps1')
$projectRoot = Get-ProjectRoot
if ([string]::IsNullOrWhiteSpace($PluginPath)) {
    $PluginPath = Get-PluginBuildBundle
}
$PluginPath = [System.IO.Path]::GetFullPath($PluginPath)
Assert-PluginBundle -BundlePath $PluginPath

$dependencyLock = Get-Content -LiteralPath (Join-Path $projectRoot 'dependencies.json') -Raw | ConvertFrom-Json
$pluginvalPath = Join-Path (Join-Path $projectRoot $dependencyLock.pluginval.directory) 'pluginval.exe'
if (-not (Test-Path -LiteralPath $pluginvalPath -PathType Leaf)) {
    throw 'pluginval is missing. Run scripts/Setup-Dependencies.ps1 -IncludePluginval.'
}
if ((Get-Item -LiteralPath $pluginvalPath).VersionInfo.ProductVersion -ne $dependencyLock.pluginval.version) {
    throw 'pluginval version differs from dependencies.json.'
}

$logDirectory = Join-Path $projectRoot 'artifacts\validation'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
$logName = 'pluginval-full.txt'
if ($SkipGuiTests) { $logName = 'pluginval-headless.txt' }
# pluginval appends to an existing named log. Start this run with an empty file
# so a previous artifact's SUCCESS cannot be mistaken for the current result.
[System.IO.File]::WriteAllText((Join-Path $logDirectory $logName), [string]::Empty)
$validationArguments = @('--strictness-level', '5', '--random-seed', '12345',
    '--sample-rates', '44100,48000,96000', '--block-sizes', '64,256,1024',
    '--output-dir', $logDirectory, '--output-filename', $logName)
if ($SkipGuiTests) { $validationArguments += '--skip-gui-tests' }
$validationArguments += @('--validate', $PluginPath)
# pluginval is a Windows GUI-subsystem executable. A pipeline makes Windows
# PowerShell wait for it; a bare invocation can return before LASTEXITCODE exists.
& $pluginvalPath @validationArguments | Out-Host
if ($LASTEXITCODE -ne 0) {
    throw ('pluginval failed with exit code {0}. Log directory: {1}' -f $LASTEXITCODE, $logDirectory)
}
Write-Output ('PASS: pluginval strictness 5. Log: ' + (Join-Path $logDirectory $logName))
