# M2 plugin scaffold evidence

Date: 2026-10-12. Target host: FL Studio 21.2.3 build 4004, Windows x64.
Source: uncommitted local main; previous files were also uncommitted. No source
commit or remote was created. Source/artifact hashes identify this checkpoint below.

## Artifact and behavior

Synthesizer v0.1.0, vendor Lazar Audio, JUCE 9.0.3 at locked revision
`be29c81492b6151c8ea8d14c840e1311963b3a83`, bundled SDK reports VST 3.8.0.
MSVC 19.44.35211/v143, VS 2022, SDK 10.0.26100.0; CMake 3.31.6-msvc6;
pluginval 1.0.4. Same installed environment as the completed M1 report.

- `SynthesizerAudioProcessor`: no audio input, stereo output, MIDI input,
  no MIDI output; clears audio and consumes MIDI in the callback.
- Fixed 560x320 JUCE editor with a silent-build message. No sound engine,
  user parameters, or saved settings. No audio-callback allocation/locks/I/O/logging.
- Component CID: `ABCDEF019182FAEB4C617A6153796261`.
- Controller CID: `ABCDEF011234ABCD4C617A6153796261`.
- Built bundle: `build/plugin/Synthesizer_artefacts/Release/VST3/Synthesizer.vst3/`.
- Staged bundle: `artifacts/VST3/Synthesizer.vst3/`.
- Installation target: `C:/Program Files/Common Files/VST3/Synthesizer.vst3/`.

## Commands and results

Run from the canonical project root `C:\Users\lazar\PycharmProjects\synthesizer`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset plugin
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Validate-Plugin.ps1 -SkipGuiTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-Plugin.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset toolchain
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset juce
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-Plugin.ps1 -Install -Validate
```

- PASS: fresh configure/build, x64 Release; no compiler warnings or errors.
- PASS: CTest 2/2 (`environment.toolchain`, `plugin.scaffold`). Product harness
  checks silent/finite dirty buffers across three sample rates and five block
  sizes (including empty/one-sample blocks), MIDI consumption, unsupported
  layouts, prepare/release, and three editor constructions/offscreen paints.
- PASS: pluginval strictness 5 without GUI tests, seed 12345, 44.1/48/96 kHz,
  64/256/1024 samples, actual VST3 discovered and cold/warm loads pass. Processing,
  bus and state callbacks pass. Parameter/automation checks have no user controls
  to test yet. Optional Steinberg standalone validator is skipped, not installed.
- PASS: staging the entire bundle with per-file SHA256 comparison. Repeated
  staging with an existing matching bundle also passes.
- PASS: all PowerShell scripts parse and CMakePresets.json parses.
- PASS: original toolchain and JUCE presets rerun after CMake/preset changes;
  CTest 1/1 and 2/2, no compiler warnings/errors.
- PASS: actual plugin binary PE Machine is AMD64 (`0x8664`).
- PASS: local Markdown links resolve, generated output paths are ignored.
- PASS: sandbox-approved installation with Windows UAC elevation; installed
  bundle verified file-by-file, administrator process exited 0. Full pluginval
  strictness 5 on the installed bundle passes, including Editor, Open editor
  whilst processing, and Editor Automation lifecycle checks. No user controls
  exist, so this does not validate future parameter automation.
- PASS: independent readback of both installed file hashes equals the build/stage
  hashes below. FL64.exe file version reconfirmed as 21.2.3.4004.
- PASS (user report, 2026-10-12): "the vst loads fine in fl studio" for this
  original scaffold. Scan settings/method were not reported.
- NOT RUN / not reported: editor reopening, multiple instances, project
  save/reopen. These need user interaction with the target host.
- NOT RUN: audio/listening, actual parameter automation/state restoration, or
  performance benchmarks. Sound engine/parameters do not exist in M2.

Raw CTest log: `build/plugin/Testing/Temporary/LastTest.log`.
Raw validator log: `artifacts/validation/pluginval-headless.txt`.
Full validation log: `artifacts/validation/pluginval-full.txt`.
Installed full run: SUCCESS. Elevated copy/validation transcript:
`artifacts/validation/install-transcript.txt`.

## Resolved script failures

- The first headless validation showed pluginval SUCCESS but the wrapper failed:
  `The variable '$LASTEXITCODE' cannot be retrieved because it has not been set.`
  pluginval is a Windows GUI executable; PowerShell's bare invocation did not
  wait. Validate-Plugin now pipes to Out-Host, waits, and checks the exit code.
  The final headless rerun exited 0 and logged SUCCESS.
- Repeated staging initially failed with `ConvertFrom-Json : Invalid JSON
  primitive: .` because the SDK-generated moduleinfo has trailing commas.
  PluginCommon now reads fixed-format 32-hex-digit CIDs without strict JSON
  parsing. The generated SDK manifest was preserved; repeated staging then passed.
- Initial root Git status hit mixed-account ownership. Inspection uses
  `git -c safe.directory=C:/Users/lazar/PycharmProjects/synthesizer`; no global
  Git configuration changes were needed.
- The first sandbox-approved installation failed at Copy-Item with
  `Access to the path 'Synthesizer.vst3' is denied.` Unsandboxed execution still
  lacked Windows administrator rights. Stage-Plugin now requests UAC elevation,
  waits/checks the elevated process, and records an installation transcript.
  The final script's staging rerun and UAC-elevated installation/full validation
  both pass. No sandbox or Windows permissions were weakened.

## Source/artifact checkpoint

SHA256 of the implementation and built artifact after the passing checks above:

| Path | SHA256 |
| --- | --- |
| CMakeLists.txt | `87db9f1a3daa96689386bf0304eb42a79c4163031f7d5524037a3dfac2ff7c55` |
| CMakePresets.json | `9207825635d9fba83bb246c8f8883e745e14a0759a3daa13a0155dc2cf05ed0e` |
| dependencies.json | `905a4b0066cb909865d90c1a476c572055150b33c81dc5b30a85bfc9a9b13ab8` |
| Source/Plugin/PluginProcessor.h | `be4ea30544d01e101842352ffdc6d276791d9b1b5180e379c1f9347749ea38dd` |
| Source/Plugin/PluginProcessor.cpp | `38fb3f1bebe3e46a9941154d1d677a7cb9f97c49bc5683c39465392316006078` |
| Source/UI/PluginEditor.h | `55b2f17f67d5990a340ae9ce0f54ec3564836a5e12d8061e699c0936fbb47d8c` |
| Source/UI/PluginEditor.cpp | `f701e02a8b84f26d5a369b9875eb282d25f531ce1a8935b671a40166dd25e41c` |
| tests/PluginScaffoldCheck.cpp | `8c9d90d7f8d684e5e607345f9671c041fb7a8725f7c10a5dca9400773a0d20d3` |
| scripts/Build-Checks.ps1 | `66dd3b58faf349633436aa05a24ce4b571f5802786aa6fb732fba98b7ec45065` |
| scripts/PluginCommon.ps1 | `684a2a3de36ace4ac3f1a16f3d936c44d9a4fdecb3b0c2e8436340969dd6cb2c` |
| scripts/Validate-Plugin.ps1 | `07ccf26b525fa143ea72cbddf8be218943bbd3360b7ed45e0bce235e0019af26` |
| scripts/Stage-Plugin.ps1 | `de4c297b2ed9ad379e8a4eeb5fcedfb25dbbd749d0c6eb2918ef56c606c20c89` |
| Contents/x86_64-win/Synthesizer.vst3 (built, staged, installed) | `5b1b80b37891a4f440344015fef80b0d9e80f47b2500cf529dd7c4acb416fa7a` |
| Contents/Resources/moduleinfo.json (built, staged, installed) | `a09c274efc437b5eb44f629ea7b1b46a03c2624aab69e9624a75d736a3131f21` |

## FL Studio acceptance

User confirmed the original scaffold loads fine in FL Studio, completing M2's
load acceptance. Additional editor/multiple-instance/project checks have not
been reported. Silence is the expected MIDI response for this build. Subsequent
LAZ-SYNTH/LAZ changes have separate evidence in [branding report](M2-branding-2026-10-12.md).
