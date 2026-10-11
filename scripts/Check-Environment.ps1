[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')

$projectRoot = Get-ProjectRoot
$visualStudioPath = Get-VisualStudioPath
$cmakePath = Get-CMakeExecutable
$gitCommand = Get-Command git.exe -ErrorAction SilentlyContinue
if (-not $gitCommand) {
    throw 'Git for Windows is missing.'
}

Write-Output ('Visual Studio 2022 C++: ' + $visualStudioPath)
Get-ChildItem -LiteralPath (Join-Path $visualStudioPath 'VC\Tools\MSVC') -Directory |
    ForEach-Object { Write-Output ('MSVC toolset: ' + $_.Name) }
Write-Output ('CMake: ' + $cmakePath)
Invoke-CheckedNative -FilePath $cmakePath -Arguments @('--version')
Invoke-CheckedNative -FilePath $gitCommand.Source -Arguments @('--version')

$sdkRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Include'
if (-not (Test-Path -LiteralPath (Join-Path $sdkRoot '10.0.26100.0\um\Windows.h'))) {
    throw 'Windows SDK 10.0.26100.0 is required by CMakePresets.json. Install it or explicitly revise the SDK pin.'
}
Get-ChildItem -LiteralPath $sdkRoot -Directory |
    ForEach-Object { Write-Output ('Windows SDK: ' + $_.Name) }

$flPath = Join-Path $env:ProgramFiles 'Image-Line\FL Studio 21\FL64.exe'
if (Test-Path -LiteralPath $flPath) {
    $flVersion = (Get-Item -LiteralPath $flPath).VersionInfo.FileVersion
    Write-Output ('FL Studio: ' + $flVersion + ' (' + $flPath + ')')
    if ($flVersion -ne '21.2.3.4004') {
        Write-Warning 'The installed FL Studio version differs from the agreed target 21.2.3.4004.'
    }
} else {
    Write-Warning 'FL Studio was not found at its default location; host testing needs an installation path.'
}

$dependencyLock = Get-Content -LiteralPath (Join-Path $projectRoot 'dependencies.json') -Raw | ConvertFrom-Json
$jucePath = Join-Path $projectRoot $dependencyLock.juce.directory
if (Test-Path -LiteralPath (Join-Path $jucePath '.git')) {
    $juceSafeDirectory = (Resolve-Path -LiteralPath $jucePath).ProviderPath.Replace('\', '/')
    $juceRevision = & $gitCommand.Source -c "safe.directory=$juceSafeDirectory" -C $jucePath rev-parse HEAD
    if ($LASTEXITCODE -ne 0 -or $juceRevision -ne $dependencyLock.juce.revision) {
        throw 'JUCE is present but does not match dependencies.json.'
    }
    $juceChanges = & $gitCommand.Source -c "safe.directory=$juceSafeDirectory" -C $jucePath status --porcelain
    if ($LASTEXITCODE -ne 0 -or $juceChanges) {
        throw 'JUCE has local changes; inspect them before treating the pinned dependency as verified.'
    }
    Write-Output ('JUCE: ' + $dependencyLock.juce.version + ' (' + $juceRevision + ')')
} else {
    Write-Output 'JUCE: MISSING; run scripts/Setup-Dependencies.ps1 from a network-enabled PowerShell.'
}
$pluginvalPath = Join-Path (Join-Path $projectRoot $dependencyLock.pluginval.directory) 'pluginval.exe'
if (Test-Path -LiteralPath $pluginvalPath) {
    $pluginvalVersion = (Get-Item -LiteralPath $pluginvalPath).VersionInfo.ProductVersion
    if ($pluginvalVersion -ne $dependencyLock.pluginval.version) {
        throw 'Installed pluginval version does not match dependencies.json; existing files were preserved.'
    }
    Write-Output ('pluginval: ' + $pluginvalVersion + ' (' + $pluginvalPath + ')')
} else {
    Write-Output 'pluginval: OPTIONAL, MISSING; run scripts/Setup-Dependencies.ps1 -IncludePluginval to install it.'
}
Write-Output 'Audit complete. Compile/runtime verification is a separate step: scripts/Build-Checks.ps1.'
