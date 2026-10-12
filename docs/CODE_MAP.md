# Project navigation

Updated: 2026-10-12. This map describes the current files; update it when paths or
responsibilities change. Start with [STATE](STATE.md) for the active task.

## Existing files

| Path from project root | Responsibility | Read when |
| --- | --- | --- |
| [AGENTS.md](../AGENTS.md) | Stable agent instructions | Session startup |
| [README.md](../README.md) | Setup, command availability, user workflow | Environment or build work |
| [docs/STATE.md](STATE.md) | Current checkpoint and next action | Session startup |
| [docs/CODE_MAP.md](CODE_MAP.md) | File navigation | Session startup |
| [docs/SPEC.md](SPEC.md) | Requirements and open product choices | Product or behavior changes |
| [docs/PLAN.md](PLAN.md) | Milestones and completion conditions | Choosing or changing work scope |
| [docs/DECISIONS.md](DECISIONS.md) | Significant decisions and rationale | Revisiting an architectural choice |
| [docs/TESTING.md](TESTING.md) | Checks and evidence requirements | Implementing or validating behavior |
| [.gitignore](../.gitignore), [.gitattributes](../.gitattributes) | Generated-file exclusions and text line endings | Source control changes |
| [dependencies.json](../dependencies.json) | JUCE version/commit and optional pluginval download pin | Dependency setup or updates |
| [CMakeLists.txt](../CMakeLists.txt) | C++17 targets, JUCE revision checks, stable VST3 identity | Build or plugin configuration |
| [CMakePresets.json](../CMakePresets.json) | VS 2022 x64 / SDK pin; toolchain, JUCE, and plugin presets | Configure/build/test changes |
| [plugin-install.json](../plugin-install.json) | User-selected physical installation directory on E: | Changing install storage |
| [scripts/Common.ps1](../scripts/Common.ps1) | Canonical Windows project path, VS/CMake discovery, native exit handling | Setup/build script changes |
| [scripts/Check-Environment.ps1](../scripts/Check-Environment.ps1) | Tool, SDK, FL executable, and dependency audit | Environment checks |
| [scripts/Setup-Dependencies.ps1](../scripts/Setup-Dependencies.ps1) | Retrieve/verify JUCE; optional curl/pluginval setup and version checks | Dependency preparation; network required |
| [scripts/Build-Checks.ps1](../scripts/Build-Checks.ps1) | Configure, build Release, then CTest for chosen preset | Running build verification |
| [scripts/PluginCommon.ps1](../scripts/PluginCommon.ps1) | Build/install paths, identity/hash checks, safe same-volume backups, junction readback | Plugin script changes |
| [scripts/Validate-Plugin.ps1](../scripts/Validate-Plugin.ps1) | Pinned pluginval strictness 5; optional GUI-test skip/path | Actual VST3 validation |
| [scripts/Stage-Plugin.ps1](../scripts/Stage-Plugin.ps1) | Workspace staging or E: install without UAC, backups/hashes/legacy retirement | Updating a built bundle |
| [scripts/Configure-PluginInstall.ps1](../scripts/Configure-PluginInstall.ps1) | One-time original relocation, E: install, per-plugin FL junction, optional validation/preview | First install/migration setup |
| [Source/Plugin/PluginProcessor.h](../Source/Plugin/PluginProcessor.h), [PluginProcessor.cpp](../Source/Plugin/PluginProcessor.cpp) | MIDI/stereo instrument lifecycle, sample-position MIDI/rendering, factory, empty state hooks | Host/audio behavior |
| [Source/DSP/SawOscillator.h](../Source/DSP/SawOscillator.h) | Phase-accumulating rising saw with PolyBLEP discontinuity correction | Oscillator/pitch/alias behavior |
| [Source/DSP/SawVoiceEngine.h](../Source/DSP/SawVoiceEngine.h), [SawVoiceEngine.cpp](../Source/DSP/SawVoiceEngine.cpp) | Eight preallocated voices, tuning/velocity/fades, FIFO note-off, queued stealing and channel panic | MIDI/voice/audio behavior |
| [Source/UI/PluginEditor.h](../Source/UI/PluginEditor.h), [PluginEditor.cpp](../Source/UI/PluginEditor.cpp) | Native fixed-size foundation editor | Editor changes |
| [tests/ToolchainSmoke.cpp](../tests/ToolchainSmoke.cpp) | Windows x64 compiler/SDK/runtime check | Toolchain failures |
| [tests/JuceModuleSmoke.cpp](../tests/JuceModuleSmoke.cpp) | JUCE audio/DSP/MIDI/parameter/GUI integration check | Module verification; not the product engine |
| [tests/PluginScaffoldCheck.cpp](../tests/PluginScaffoldCheck.cpp) | Product saw/timing/voices/panic/state checks plus layouts/lifecycle/editor harness | Product regression checks; CTest plugin.scaffold |
| [docs/validation/M3-fixed-saw-2026-10-12.md](validation/M3-fixed-saw-2026-10-12.md) | Fixed saw build, behavior/artifact evidence, and pending FL checks | Auditing M3 evidence |
| [docs/validation/M1-2026-10-11.md](validation/M1-2026-10-11.md) | Exact setup-check results and source/artifact hashes | Auditing M1 evidence |
| [docs/validation/M1-environment-complete-2026-10-11.md](validation/M1-environment-complete-2026-10-11.md) | Completed environment evidence, resolved failures, final source/artifact hashes | Auditing installed M1 environment |
| [docs/validation/M2-plugin-scaffold-2026-10-12.md](validation/M2-plugin-scaffold-2026-10-12.md) | Original scaffold build/validation and user-confirmed FL loading | Auditing the original scaffold |
| [docs/validation/M2-branding-2026-10-12.md](validation/M2-branding-2026-10-12.md) | LAZ-SYNTH/LAZ branding, stable CIDs, relocation/validation, user-confirmed FL opening | Auditing the renamed scaffold |

## Implementation entry points

`createPluginFilter` constructs `SynthesizerAudioProcessor`. Its `processBlock`
clears output, renders the voice engine between MIDI event positions, and consumes
MIDI; `createEditor` constructs `SynthesizerAudioProcessorEditor`.
`PluginScaffoldCheck::main` exercises the same shared processor/editor code as
the wrapper. Environment entry points remain `ToolchainSmoke::main` and
`JuceModuleSmoke::main`. `SawVoiceEngine` owns the fixed oscillator/fade pool;
`SawOscillator::nextSample` generates each voice's saw. No parameters exist yet. Command usage
is in README; current validation/host status is in STATE.

Generated paths (ignored by Git):

- `build/toolchain/Release/ToolchainSmoke.exe`: verified x64 check executable.
- `build/toolchain/Testing/Temporary/LastTest.log`: local CTest runtime evidence.
- `build/juce/JuceModuleSmoke_artefacts/Release/JuceModuleSmoke.exe`: JUCE module check.
- `build/juce/Testing/Temporary/LastTest.log`: both environment tests' evidence.
- `build/juce-before-path-fix/`: preserved generated build from before path normalization.
- `build/plugin/Synthesizer_artefacts/Release/VST3/LAZ-SYNTH.vst3/`: current built bundle.
- `build/plugin/PluginScaffoldCheck_artefacts/Release/PluginScaffoldCheck.exe`: product harness.
- `build/plugin/Testing/Temporary/LastTest.log`: plugin preset's two CTest checks.
- `artifacts/VST3/LAZ-SYNTH.vst3/`: staged bundle; every copied file is hash-checked.
- `artifacts/validation/pluginval-headless.txt`, `pluginval-full.txt`: latest validator logs.
- `artifacts/validation/install-transcript.txt`: latest physical-install transcript.
- `artifacts/validation/install-setup-transcript.txt`: one-time migration/junction transcript.
- `artifacts/install-backups/`: preserved matching bundles from installation updates.
- `third_party/JUCE/`: clean pinned JUCE 9.0.3 checkout, including its VST3 SDK headers.
- `tools/pluginval-1.0.4/pluginval.exe`: installed plugin validator (version/help checked).
- `tools/downloads/pluginval-1.0.4-Windows.zip`: downloaded official release archive.

Physical install: `E:\everything_music_related\VstPlugins\LAZ-SYNTH.vst3`.
FL scan entry: `C:\Program Files\Common Files\VST3\LAZ-SYNTH.vst3`, a junction to
the E: bundle. Legacy C: bundles move to workspace backups; legacy E: bundles
move to `E:\everything_music_related\VstPlugins\.laz-synth-backups` after checks.
The internal CMake target remains `Synthesizer`.
See STATE for whether installation/host acceptance passed for the current binary.
Inspect mapped paths first; broaden searches when the task or evidence requires it.
