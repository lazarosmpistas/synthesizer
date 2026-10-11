[CmdletBinding()]
param(
    [switch]$IncludePluginval
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')
$projectRoot = Get-ProjectRoot
$dependencyLock = Get-Content -LiteralPath (Join-Path $projectRoot 'dependencies.json') -Raw | ConvertFrom-Json
$gitCommand = Get-Command git.exe -ErrorAction SilentlyContinue
if (-not $gitCommand) {
    throw 'Git for Windows is required to download and verify JUCE.'
}
$jucePath = Join-Path $projectRoot $dependencyLock.juce.directory
New-Item -ItemType Directory -Path (Split-Path -Parent $jucePath) -Force | Out-Null

if (-not (Test-Path -LiteralPath $jucePath)) {
    Invoke-CheckedNative -FilePath $gitCommand.Source -Arguments @(
        'clone', '--depth', '1', '--branch', $dependencyLock.juce.version,
        $dependencyLock.juce.repository, $jucePath
    )
}
if (-not (Test-Path -LiteralPath (Join-Path $jucePath '.git'))) {
    throw 'Existing third_party/JUCE is not a Git checkout. It was preserved; inspect it before retrying.'
}
$juceSafeDirectory = (Resolve-Path -LiteralPath $jucePath).ProviderPath.Replace('\', '/')
$juceChanges = & $gitCommand.Source -c "safe.directory=$juceSafeDirectory" -C $jucePath status --porcelain
if ($LASTEXITCODE -ne 0 -or $juceChanges) {
    throw 'JUCE has local changes. They were preserved; inspect them before retrying.'
}
$juceRevision = & $gitCommand.Source -c "safe.directory=$juceSafeDirectory" -C $jucePath rev-parse HEAD
if ($LASTEXITCODE -ne 0 -or $juceRevision -ne $dependencyLock.juce.revision) {
    throw 'JUCE revision does not match the lock file. The checkout was preserved; do not use it as the pinned dependency.'
}
Write-Output ('Verified JUCE ' + $dependencyLock.juce.version + ' at ' + $juceRevision)

if ($IncludePluginval) {
    $pluginvalPath = Join-Path $projectRoot $dependencyLock.pluginval.directory
    $pluginvalExecutable = Join-Path $pluginvalPath 'pluginval.exe'
    if (Test-Path -LiteralPath $pluginvalPath) {
        if (-not (Test-Path -LiteralPath $pluginvalExecutable)) {
            throw 'Existing pluginval directory is incomplete; inspect it before retrying. It was preserved.'
        }
        Write-Output ('Using existing pluginval at ' + $pluginvalExecutable)
    } else {
        $downloadDirectory = Join-Path $projectRoot 'tools/downloads'
        New-Item -ItemType Directory -Path $downloadDirectory -Force | Out-Null
        $archivePath = Join-Path $downloadDirectory ('pluginval-' + $dependencyLock.pluginval.version + '-Windows.zip')
        $curlCommand = Get-Command curl.exe -ErrorAction SilentlyContinue
        if (-not $curlCommand) {
            throw 'Windows curl.exe is required to download pluginval.'
        }
        Invoke-CheckedNative -FilePath $curlCommand.Source -Arguments @(
            '--fail', '--location', '--silent', '--show-error', '--retry', '2',
            '--connect-timeout', '20', '--max-time', '180',
            '--output', $archivePath, $dependencyLock.pluginval.download
        )
        Write-Output ('Downloaded archive SHA256: ' + (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash)
        Expand-Archive -LiteralPath $archivePath -DestinationPath $pluginvalPath
        if (-not (Test-Path -LiteralPath $pluginvalExecutable)) {
            throw 'The downloaded archive does not contain pluginval.exe at the expected location.'
        }
    }
    $pluginvalVersion = (Get-Item -LiteralPath $pluginvalExecutable).VersionInfo.ProductVersion
    if ($pluginvalVersion -ne $dependencyLock.pluginval.version) {
        throw 'Installed pluginval version does not match dependencies.json; existing files were preserved.'
    }
    Write-Output ('Verified pluginval ' + $pluginvalVersion + ' at ' + $pluginvalExecutable + '; plugin validation requires a VST3 artifact.')
}

Write-Output 'Dependency setup finished. Run scripts/Build-Checks.ps1 -Preset juce to verify compilation and runtime.'
