[CmdletBinding()]
param(
    [switch]$Install,
    [switch]$Validate
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'PluginCommon.ps1')
$projectRoot = Get-ProjectRoot
$sourceBundle = Get-PluginBuildBundle
Assert-PluginBundle -BundlePath $sourceBundle

$logDirectory = Join-Path $projectRoot 'artifacts\validation'
if ($Install) {
    New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
    Start-Transcript -Path (Join-Path $logDirectory 'install-transcript.txt') -Force | Out-Null
}

try {
    $destinationRoot = Join-Path $projectRoot 'artifacts\VST3'
    if ($Install) {
        # The actual files live on the user's selected drive. One-time setup
        # creates a standard-folder junction for FL Studio's VST3 scanner.
        $destinationRoot = Get-PluginInstallRoot
        $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = New-Object Security.Principal.WindowsPrincipal($identity)
        Write-Output ('Physical install: {0}; administrator token: {1}' -f $destinationRoot,
            $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))
    }
    $destinationBundle = Join-Path $destinationRoot (Split-Path -Leaf $sourceBundle)
    $legacyBundle = Join-Path $destinationRoot 'Synthesizer.vst3'
    foreach ($existingBundle in @($destinationBundle, $legacyBundle)) {
        if (Test-Path -LiteralPath $existingBundle) {
            Assert-SamePluginIdentity -SourceBundle $sourceBundle -ExistingBundle $existingBundle
            if ((Get-Item -LiteralPath $existingBundle).Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
                throw ('Expected a physical bundle at ' + $existingBundle + '; link was preserved.')
            }
        }
    }
    if ($Install -and (Test-Path -LiteralPath $destinationBundle)) {
        $backupRoot = Join-Path $projectRoot ('artifacts\install-backups\' + [guid]::NewGuid().ToString('N'))
        New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
        Copy-Item -LiteralPath $destinationBundle -Destination $backupRoot -Recurse
        Write-Output ('Previous installed bundle backed up at ' + $backupRoot)
    }

    New-Item -ItemType Directory -Path $destinationRoot -Force | Out-Null
    Copy-Item -LiteralPath $sourceBundle -Destination $destinationRoot -Recurse -Force
    Assert-PluginCopy -SourceBundle $sourceBundle -DestinationBundle $destinationBundle
    Write-Output ('PASS: copied and verified the VST3 bundle at ' + $destinationBundle)
    if ($Validate) {
        & (Join-Path $PSScriptRoot 'Validate-Plugin.ps1') -PluginPath $destinationBundle
    }

    # Retire the old filename only after the replacement copy/validation succeeds.
    # Keep its complete bundle outside the scanner so only one copy of this CID remains.
    if (Test-Path -LiteralPath $legacyBundle) {
        $legacyBackupPath = Move-PluginBundleToBackup -BundlePath $legacyBundle -ExpectedParent $destinationRoot
        Write-Output ('PASS: previous Synthesizer bundle preserved at ' + $legacyBackupPath)
    }
} finally {
    if ($Install) { Stop-Transcript | Out-Null }
}
