. (Join-Path $PSScriptRoot 'Common.ps1')

function Get-PluginBuildBundle {
    return (Join-Path (Get-ProjectRoot) 'build\plugin\Synthesizer_artefacts\Release\VST3\LAZ-SYNTH.vst3')
}

function Get-PluginInstallRoot {
    $configuration = Get-Content -LiteralPath (Join-Path (Get-ProjectRoot) 'plugin-install.json') -Raw | ConvertFrom-Json
    if (-not [System.IO.Path]::IsPathRooted($configuration.directory)) {
        throw 'plugin-install.json must specify an absolute installation directory.'
    }
    return [System.IO.Path]::GetFullPath($configuration.directory).TrimEnd('\', '/')
}

function Assert-PluginBundle {
    param([Parameter(Mandatory = $true)][string]$BundlePath)
    $binaryName = Split-Path -Leaf $BundlePath.TrimEnd('\', '/')
    if (-not (Test-Path -LiteralPath (Join-Path $BundlePath ('Contents\x86_64-win\' + $binaryName)) -PathType Leaf) -or
        -not (Test-Path -LiteralPath (Join-Path $BundlePath 'Contents\Resources\moduleinfo.json') -PathType Leaf)) {
        throw ('Expected a complete Windows x64 VST3 bundle at {0}. Build with Build-Checks.ps1 -Preset plugin.' -f $BundlePath)
    }
}

function Get-PluginClassIds {
    param([Parameter(Mandatory = $true)][string]$BundlePath)
    # The SDK-generated moduleinfo.json has trailing commas, which Windows
    # PowerShell 5.1's JSON parser rejects. Read only the fixed-format class IDs.
    $manifest = Get-Content -LiteralPath (Join-Path $BundlePath 'Contents\Resources\moduleinfo.json') -Raw
    $classIds = @([regex]::Matches($manifest, '(?m)^\s*"CID"\s*:\s*"([0-9a-fA-F]{32})"') |
        ForEach-Object { $_.Groups[1].Value.ToUpperInvariant() } | Sort-Object -Unique)
    if ($classIds.Count -eq 0) {
        throw ('No VST3 class IDs found in ' + $BundlePath)
    }
    return $classIds
}

function Assert-SamePluginIdentity {
    param([string]$SourceBundle, [string]$ExistingBundle)
    Assert-PluginBundle -BundlePath $SourceBundle
    Assert-PluginBundle -BundlePath $ExistingBundle
    $sourceIds = @(Get-PluginClassIds -BundlePath $SourceBundle)
    $existingIds = @(Get-PluginClassIds -BundlePath $ExistingBundle)
    if (($sourceIds -join ',') -ne ($existingIds -join ',')) {
        throw ('An unrelated plugin exists at {0}; it was preserved.' -f $ExistingBundle)
    }
}

function Assert-PluginCopy {
    param([string]$SourceBundle, [string]$DestinationBundle)
    Assert-PluginBundle -BundlePath $DestinationBundle
    foreach ($sourceFile in Get-ChildItem -LiteralPath $SourceBundle -Recurse -File) {
        $relativePath = $sourceFile.FullName.Substring($SourceBundle.Length + 1)
        $destinationFile = Join-Path $DestinationBundle $relativePath
        if ((Get-FileHash -LiteralPath $sourceFile.FullName -Algorithm SHA256).Hash -ne
            (Get-FileHash -LiteralPath $destinationFile -Algorithm SHA256).Hash) {
            throw ('Copied bundle hash mismatch: ' + $relativePath)
        }
    }
}

function Move-PluginBundleToBackup {
    param([string]$BundlePath, [string]$ExpectedParent)
    $sourcePath = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $BundlePath).ProviderPath)
    $bundleName = Split-Path -Leaf $sourcePath
    $expectedPath = [System.IO.Path]::GetFullPath((Join-Path $ExpectedParent $bundleName))
    $backupParent = [System.IO.Path]::GetFullPath((Join-Path (Get-ProjectRoot) 'artifacts\install-backups'))
    # Directory Move-Item stays on one volume. E: installs archive their legacy
    # bundle beside the physical install; C: bundles archive inside the workspace.
    if ([System.IO.Path]::GetPathRoot($sourcePath) -ne [System.IO.Path]::GetPathRoot($backupParent)) {
        $backupParent = [System.IO.Path]::GetFullPath((Join-Path $ExpectedParent '.laz-synth-backups'))
    }
    $backupRoot = Join-Path $backupParent ([guid]::NewGuid().ToString('N'))
    $backupPath = [System.IO.Path]::GetFullPath((Join-Path $backupRoot $bundleName))
    if ($bundleName -notin @('Synthesizer.vst3', 'LAZ-SYNTH.vst3') -or
        $sourcePath -ne $expectedPath -or
        -not $backupPath.StartsWith($backupParent + '\', [StringComparison]::OrdinalIgnoreCase) -or
        ((Get-Item -LiteralPath $sourcePath).Attributes -band [System.IO.FileAttributes]::ReparsePoint)) {
        throw 'Bundle move failed absolute path checks; existing bundle preserved.'
    }
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    try {
        Move-Item -LiteralPath $sourcePath -Destination $backupPath
    } catch {
        throw ('Could not move the previous bundle to backup. Close plugin instances in FL Studio and retry. Original error: ' + $_.Exception.Message)
    }
    return $backupPath
}

function Test-PluginInstallJunction {
    param([string]$JunctionPath, [string]$ExpectedTarget)
    $junction = Get-Item -LiteralPath $JunctionPath -Force -ErrorAction SilentlyContinue
    if ($null -eq $junction -or $junction.LinkType -ne 'Junction') { return $false }
    $targets = @($junction.Target)
    return $targets.Count -eq 1 -and
        [System.IO.Path]::GetFullPath($targets[0]).TrimEnd('\', '/') -eq
        [System.IO.Path]::GetFullPath($ExpectedTarget).TrimEnd('\', '/')
}
