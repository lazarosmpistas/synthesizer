Set-StrictMode -Version Latest

function Get-ProjectRoot {
    # Git's ownership exception compares actual Windows path casing. Resolve-Path
    # preserves the caller's spelling, so resolve this file through its handle.
    if (-not ('SynthEnvironment.NativePath' -as [type])) {
        Add-Type -TypeDefinition @'
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Win32.SafeHandles;
namespace SynthEnvironment {
    public static class NativePath {
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern uint GetFinalPathNameByHandle(
            SafeFileHandle handle, StringBuilder path, uint capacity, uint flags);
    }
}
'@
    }
    $scriptStream = [System.IO.File]::OpenRead((Join-Path $PSScriptRoot 'Common.ps1'))
    try {
        $canonicalPath = New-Object System.Text.StringBuilder 32768
        $pathLength = [SynthEnvironment.NativePath]::GetFinalPathNameByHandle(
            $scriptStream.SafeFileHandle, $canonicalPath, $canonicalPath.Capacity, 0)
        if ($pathLength -eq 0) {
            throw (New-Object System.ComponentModel.Win32Exception)
        }
        if ($pathLength -ge $canonicalPath.Capacity) {
            throw 'The canonical project path exceeds the Windows path limit.'
        }
        $scriptPath = $canonicalPath.ToString()
        if ($scriptPath.StartsWith('\\?\UNC\')) {
            $scriptPath = '\\' + $scriptPath.Substring(8)
        } elseif ($scriptPath.StartsWith('\\?\')) {
            $scriptPath = $scriptPath.Substring(4)
        }
        return (Split-Path -Parent (Split-Path -Parent $scriptPath))
    } finally {
        $scriptStream.Dispose()
    }
}

function Get-VisualStudioPath {
    $vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswherePath)) {
        throw 'Visual Studio Installer is missing. Install Visual Studio 2022 with Desktop development with C++.'
    }
    $installationPath = & $vswherePath -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($installationPath)) {
        throw 'Visual Studio 2022 C++ tools are missing. Add Desktop development with C++ in Visual Studio Installer.'
    }
    return $installationPath.Trim()
}

function Get-CMakeExecutable {
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmakeCommand) {
        return $cmakeCommand.Source
    }
    $visualStudioPath = Get-VisualStudioPath
    $cmakePath = Join-Path $visualStudioPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (-not (Test-Path -LiteralPath $cmakePath)) {
        throw 'CMake is missing. Add C++ CMake tools for Windows in Visual Studio Installer (CMake >= 3.25).'
    }
    return $cmakePath
}

function Invoke-CheckedNative {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$Arguments
    )
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw ('Command failed with exit code {0}: {1} {2}' -f $LASTEXITCODE, $FilePath, ($Arguments -join ' '))
    }
}
