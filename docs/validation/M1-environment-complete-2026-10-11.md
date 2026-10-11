# M1 completed environment validation - 2026-10-11

This report supersedes the readiness results in the historical
[initial checkpoint](M1-2026-10-11.md). It applies to the source and artifact
hashes below. Commands ran from `C:\Users\lazar\PycharmProjects\synthesizer`
in Windows PowerShell. The local Git repository remains on main with uncommitted
source and no remote; SHA256 identifies the checked checkpoint.

## Installed environment

- Existing Visual Studio Community 2022 17.14.8, MSVC 19.44.35211/v143
  (tools directory 14.44.35207), MSBuild 17.14.14.31908.
- Selected Windows SDK 10.0.26100.0; CMake/CTest 3.31.6-msvc6; Git
  2.45.1.windows.1. No global PATH or execution-policy changes were needed.
- FL Studio x64 file version 21.2.3.4004 at
  `C:\Program Files\Image-Line\FL Studio 21\FL64.exe`.
- JUCE 9.0.3 at clean commit `be29c81492b6151c8ea8d14c840e1311963b3a83`,
  cloned into `third_party/JUCE`; verified against dependencies.json and the
  [official release](https://github.com/juce-framework/JUCE/releases/tag/9.0.3).
- JUCE's bundled VST3 sources/headers are present in
  `modules/juce_audio_processors_headless/format_types/VST3_SDK`; a separate SDK
  installation is unnecessary for the initial JUCE plugin target.
- pluginval 1.0.4, downloaded from the
  [official release](https://github.com/Tracktion/pluginval/releases/tag/v1.0.4)
  into `tools/pluginval-1.0.4/pluginval.exe`. Embedded file/product version and
  command-line version are 1.0.4. Windows curl.exe 8.21.0 performed the download.
- Archive SHA256 was recorded locally. The release metadata has no upstream
  checksum; no upstream checksum comparison or plugin validation is claimed.

## Verified commands and outcomes

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-Dependencies.ps1 -IncludePluginval
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Check-Environment.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset toolchain
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset juce
& .\tools\pluginval-1.0.4\pluginval.exe --version | Out-String
& .\tools\pluginval-1.0.4\pluginval.exe --help | Out-String
```

PASS: Dependency setup downloaded the exact JUCE tag within the sandbox.
After the fixes below, the setup script downloaded/extracted pluginval outside
the sandbox. The final script rerun inside the sandbox verified existing clean
JUCE and pluginval's version, preserved both directories, and exited 0.

PASS: Final environment audit exited 0, reported the required SDK/tools and
target FL version, verified clean JUCE HEAD, and matched pluginval to the lock.

PASS: Offline toolchain preset configured/built x64 Release and passed 1/1 tests.
JUCE preset configured from a fresh build directory, compiled all selected
modules, and passed 2/2 tests. No compiler or MSBuild warnings appeared in the
final fresh JUCE build. Output:

```text
PASS: Windows x64, MSVC 194435211, C++17, Windows SDK/runtime, lock-free float atomics, finite offline samples.
PASS: JUCE v9.0.3 audio, DSP, voice/MIDI, parameter, and GUI modules linked and ran.
100% tests passed, 0 tests failed out of 2
```

PASS: pluginval --version printed `pluginval - 1.0.4`, and --help printed its
validation options. Both commands exited 0. These checks establish executable
availability; no product VST3 exists to validate yet.

Final local runtime evidence:

- `build/toolchain/Testing/Temporary/LastTest.log`.
- `build/juce/Testing/Temporary/LastTest.log`.

## Failures encountered and resolved

1. Initial Setup-Dependencies.ps1 -IncludePluginval cloned and verified JUCE,
   then Invoke-WebRequest failed with `The underlying connection was closed:
   An unexpected error occurred on a receive.`; script exited 1.
2. The required retry outside the sandbox exposed Git's ownership protection:
   JUCE was owned by CodexSandboxOnline, while the process ran as lazar. Root
   Git inspection similarly needed a per-command exception for the user-owned
   workspace. A Resolve-Path exception still failed because it preserved the
   caller's lowercase `pycharmprojects` rather than actual `PycharmProjects`.
   Common.ps1 now obtains the canonical project spelling through
   GetFinalPathNameByHandle. Setup, audit, and CMake trust only the JUCE path for
   each Git command, then enforce clean status and exact revision. No global
   safe.directory setting or wildcard trust was added.
3. Windows curl downloaded the official archive successfully outside the sandbox:

   ```powershell
   curl.exe --fail --location --retry 2 --connect-timeout 20 --max-time 180 --output tools/downloads/pluginval-1.0.4-Windows.zip https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip
   ```

   Setup now uses checked curl with redirects, retry and timeout options instead
   of Invoke-WebRequest. Its full download/extraction path then passed.
4. Reconfiguring the earlier lowercase-path JUCE build emitted MSB8028 for stale
   intermediate files. The directory was moved with PowerShell Move-Item to
   `build/juce-before-path-fix` after checking source/destination stayed inside
   the workspace. A fresh build at canonical `build/juce` passed without warnings.
   The earlier generated build was preserved; no dependency source was modified.

## Source/configuration and documentation checks

PASS: All four scripts parsed with
`System.Management.Automation.Language.Parser::ParseFile`; dependencies.json
and CMakePresets.json parsed with `Get-Content -Raw | ConvertFrom-Json`.

```powershell
git -c safe.directory=C:/Users/lazar/PycharmProjects/synthesizer/third_party/JUCE -C third_party/JUCE status --porcelain
git -c safe.directory=C:/Users/lazar/PycharmProjects/synthesizer check-ignore build/juce/JuceModuleSmoke_artefacts/Release/JuceModuleSmoke.exe third_party/JUCE/CMakeLists.txt tools/pluginval-1.0.4/pluginval.exe artifacts/staged.vst3 .vs/session .idea/workspace.xml CMakeUserPresets.json
git -c safe.directory=C:/Users/lazar/PycharmProjects/synthesizer ls-files --others --exclude-standard
```

PASS: JUCE status was empty. All seven generated/local paths were ignored.
Source enumeration contained only intended project files. Final relative
Markdown links, CODE_MAP file paths, and STATE's maximum-80-line size were
checked before handoff.

## Checked source and artifact hashes

Exact hash command: `Get-FileHash -LiteralPath <path> -Algorithm SHA256`.
Documentation edits afterward do not change these checked build sources.

| Path | SHA256 |
| --- | --- |
| `CMakeLists.txt` | `FC49FE945F39AF1A26B3D0A52DA7479198D5FCD1A8E6DF3965CDE96C63CAA9B4` |
| `CMakePresets.json` | `B8D7ADAD252158303B4642AE00054DD50998A66459B2F4459B5DB7B88BAABC1F` |
| `dependencies.json` | `905A4B0066CB909865D90C1A476C572055150B33C81DC5B30A85BFC9A9B13AB8` |
| `scripts/Common.ps1` | `14AC23DF0D3842748BD36F62E3D74A585BFD844A83D42D22CBEAC3A944922ABC` |
| `scripts/Check-Environment.ps1` | `2A9D7595F7A3C940C3A1B080C06CEC3518C30D3E2D674665F5FC678FBBF67E00` |
| `scripts/Build-Checks.ps1` | `733B3CCC5CFC6698E4374DFBCA7623B8517365861743B631A64F3CD75D150B9E` |
| `scripts/Setup-Dependencies.ps1` | `B0388F4BF7D02794E20DB525AF85F215E7CD888FB8186E7F870A71B7D2817BF5` |
| `tests/ToolchainSmoke.cpp` | `BE899D50227828112117834979C681760E488C0F8DA65FCBB84EB57ADB685B6F` |
| `tests/JuceModuleSmoke.cpp` | `B264FDA134C07BBB3DC55F3B2ED9D585D62E60F6B1E1F80D2436FBDBECB35AA2` |
| `build/toolchain/Release/ToolchainSmoke.exe` | `298262DAC78D9BFE497EBADF304357BCF4FD1C15683C8862F9F5E4DE6201B274` |
| `build/juce/Release/ToolchainSmoke.exe` | `5E0AB6D2D86426127A92455D4D278E08E0695AFA466CCFB947CA40562FEEFA95` |
| `build/juce/JuceModuleSmoke_artefacts/Release/JuceModuleSmoke.exe` | `E1DF096051375C668D2364728FCE6A158AE3DB29EE01FD888A1C077C23B9296A` |
| `tools/pluginval-1.0.4/pluginval.exe` | `F4AC5C31C5544A434F73E686B816861D92C22AE849D69AD693681CBC9522FCD8` |
| `tools/downloads/pluginval-1.0.4-Windows.zip` | `C08E61CE3B96DB41636F8EC7E76F4C7E2C13EBDAC7FA1B5A1F52B4F32EC715AB` |

## Scope and next milestone

M1 is complete. No product VST3 build, pluginval validation of a plugin, FL
Studio scan/load/playback, listening, automation/state restoration, or performance
measurement ran: a product target does not exist yet. These remain separate
M2/M3 acceptance checks. Next, follow [PLAN M2](../PLAN.md) to create the minimal
VST3 instrument and documented build/validate/stage/host-loading workflow.
