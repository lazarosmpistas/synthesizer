[CmdletBinding()]
param(
    [switch]$Validate,
    [switch]$Preview
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'PluginCommon.ps1')
$projectRoot = Get-ProjectRoot
$sourceBundle = Get-PluginBuildBundle
Assert-PluginBundle -BundlePath $sourceBundle
$installRoot = Get-PluginInstallRoot
$installedBundle = Join-Path $installRoot 'LAZ-SYNTH.vst3'
$standardRoot = Join-Path $env:CommonProgramFiles 'VST3'
$standardBundle = Join-Path $standardRoot 'LAZ-SYNTH.vst3'
$originalBundle = Join-Path $standardRoot 'Synthesizer.vst3'
$relocatedOriginal = Join-Path $installRoot 'Synthesizer.vst3'

foreach ($existingBundle in @($installedBundle, $originalBundle, $relocatedOriginal)) {
    if (Test-Path -LiteralPath $existingBundle) {
        Assert-SamePluginIdentity -SourceBundle $sourceBundle -ExistingBundle $existingBundle
        if ((Get-Item -LiteralPath $existingBundle).Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
            throw ('Expected a physical bundle at ' + $existingBundle + '; existing link was preserved.')
        }
    }
}
$linkIsReady = Test-PluginInstallJunction -JunctionPath $standardBundle -ExpectedTarget $installedBundle
if ((Get-Item -LiteralPath $standardBundle -Force -ErrorAction SilentlyContinue) -and -not $linkIsReady) {
    throw ('An unexpected bundle/link exists at ' + $standardBundle + '; it was preserved.')
}
if ($Preview) {
    Write-Output ('Storage: ' + $installedBundle)
    Write-Output ('FL Studio junction: ' + $standardBundle)
    Write-Output ('Original to relocate first (if present): ' + $originalBundle)
    return
}

if (-not $linkIsReady -or (Test-Path -LiteralPath $originalBundle)) {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        $setupArguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', ('"' + $PSCommandPath + '"'))
        if ($Validate) { $setupArguments += '-Validate' }
        Write-Output 'Requesting one-time administrator access to migrate the old install and create the FL Studio junction.'
        $setupProcess = Start-Process -FilePath 'powershell.exe' -Verb RunAs -WindowStyle Hidden -Wait -PassThru -ArgumentList $setupArguments
        if ($setupProcess.ExitCode -ne 0) {
            throw ('Install setup failed with exit code {0}. See artifacts/validation/install-setup-transcript.txt.' -f $setupProcess.ExitCode)
        }
        Write-Output 'PASS: one-time installation setup completed.'
        return
    }
}

$logDirectory = Join-Path $projectRoot 'artifacts\validation'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
Start-Transcript -Path (Join-Path $logDirectory 'install-setup-transcript.txt') -Force | Out-Null
try {
    # Relocate the original bytes to E: before installing the renamed build.
    # Keep the working C: copy until the replacement validates successfully.
    if (Test-Path -LiteralPath $originalBundle) {
        if (-not (Test-Path -LiteralPath $relocatedOriginal)) {
            New-Item -ItemType Directory -Path $installRoot -Force | Out-Null
            Copy-Item -LiteralPath $originalBundle -Destination $installRoot -Recurse
        }
        Assert-PluginCopy -SourceBundle $originalBundle -DestinationBundle $relocatedOriginal
        Write-Output ('PASS: original bundle relocated and verified at ' + $relocatedOriginal)
    }

    & (Join-Path $PSScriptRoot 'Stage-Plugin.ps1') -Install -Validate:$Validate

    if (-not $linkIsReady) {
        New-Item -ItemType Directory -Path $standardRoot -Force | Out-Null
        New-Item -ItemType Junction -Path $standardBundle -Target $installedBundle | Out-Null
    }
    if (-not (Test-PluginInstallJunction -JunctionPath $standardBundle -ExpectedTarget $installedBundle)) {
        throw 'Standard-folder junction does not point to the selected physical installation.'
    }
    Assert-PluginCopy -SourceBundle $sourceBundle -DestinationBundle $standardBundle
    if ($Validate) {
        & (Join-Path $PSScriptRoot 'Validate-Plugin.ps1') -PluginPath $standardBundle -SkipGuiTests
    }
    if (Test-Path -LiteralPath $originalBundle) {
        $oldBackup = Move-PluginBundleToBackup -BundlePath $originalBundle -ExpectedParent $standardRoot
        Write-Output ('PASS: original Program Files bundle preserved at ' + $oldBackup)
    }
    Write-Output ('PASS: FL Studio path links to E: storage: ' + $standardBundle + ' -> ' + $installedBundle)
} finally {
    Stop-Transcript | Out-Null
}
