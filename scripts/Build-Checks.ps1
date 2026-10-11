[CmdletBinding()]
param(
    [ValidateSet('toolchain', 'juce', 'plugin')][string]$Preset = 'toolchain'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')
$cmakePath = Get-CMakeExecutable
$ctestPath = Join-Path (Split-Path -Parent $cmakePath) 'ctest.exe'
if (-not (Test-Path -LiteralPath $ctestPath)) {
    throw 'CTest is missing from the CMake installation.'
}

Push-Location -LiteralPath (Get-ProjectRoot)
try {
    Invoke-CheckedNative -FilePath $cmakePath -Arguments @('--preset', $Preset)
    Invoke-CheckedNative -FilePath $cmakePath -Arguments @('--build', '--preset', ($Preset + '-release'), '--parallel', '2')
    Invoke-CheckedNative -FilePath $ctestPath -Arguments @('--preset', ($Preset + '-release'), '--verbose')
} finally {
    Pop-Location
}
