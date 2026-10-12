# LAZ-SYNTH

A Windows x64 VST3 synthesizer project for **FL Studio 21.2.3 build 4004**, using
C++17, JUCE, and CMake. Private use is the current goal. The development
environment is installed. **LAZ-SYNTH v0.1.0**, vendor **LAZ**, is a minimal VST3 instrument
with a native JUCE editor, MIDI input, and stereo output. It now plays a fixed
saw sound: eight voices, velocity response, and short internal note fades.
There are no effects, filters, or sound controls yet.

## Build and open the plugin

Run from the project root in PowerShell:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset plugin
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Validate-Plugin.ps1 -SkipGuiTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-Plugin.ps1
```

These commands have passed: x64 Release build, CTest 2/2, pluginval strictness 5
without GUI tests, and staging with SHA256 comparisons (including repeated staging).
The renamed E: installation also passes full GUI validation and native loading
through its standard-folder junction. Physical updates and repeated setup were
verified with a non-admin token; see STATE for the exact checkpoint.
The processor check exercises saw pitch/harmonics, alias reduction, fixed fades,
velocity, MIDI timing, repeated notes/channels, eight voices/stealing, and panic
messages at 44.1/48/96 kHz and 0/1/64/256/1024 samples. It retains layout rejection,
lifecycle reset, and editor recreation/offscreen painting.
pluginval loads the actual VST3 bundle and checks processing at 64/256/1024 samples.
It has no user parameters or persistent settings yet; its state
callbacks are empty. Passing state/automation checks does not verify future controls.

The build produces this **folder bundle**:

```text
build\plugin\Synthesizer_artefacts\Release\VST3\LAZ-SYNTH.vst3
```

Staging copies it to `artifacts\VST3\LAZ-SYNTH.vst3`. Copy the entire folder,
including `Contents\Resources\moduleinfo.json` and
`Contents\x86_64-win\LAZ-SYNTH.vst3`; the inner binary alone is not the bundle.
Builds never install plugins automatically.

Close any instances of LAZ-SYNTH (or its original Synthesizer build) before an
update. Actual plugin files live in **E:\everything_music_related\VstPlugins**,
as selected by the user in [plugin-install.json](plugin-install.json).
Perform the one-time relocation and FL Studio directory-link setup with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Configure-PluginInstall.ps1 -Validate
```

This first copies/verifies the original Synthesizer bundle on E:, then installs
the renamed build at `E:\everything_music_related\VstPlugins\LAZ-SYNTH.vst3`.
It validates that physical bundle, creates a per-plugin directory junction
`C:\Program Files\Common Files\VST3\LAZ-SYNTH.vst3` pointing to it, and verifies
native plugin loading through the junction. Only that plugin's standard-folder
entry is changed. The initial migration/link setup requests administrator access
through UAC; builds and future plugin updates use the user-selected drive.

After setup, update and validate the installation without Windows administrator
elevation using:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-Plugin.ps1 -Install -Validate
```

Each copied file is SHA256 checked. Existing bundle class IDs must match;
unrelated plugins are preserved. Existing matching installs are backed up in
`artifacts\install-backups`. Legacy C: bundles move there after the replacement
validates. Legacy E: bundles move into `VstPlugins\.laz-synth-backups` on the
same volume. No recursive deletion is used. The standard folder contains a
link; the plugin bytes remain on E:.
VST3 component/controller IDs stay unchanged. Rescan previously verified plugins
in FL Studio to refresh the name and vendor. Project save/reopen compatibility
after this rename still needs a host check.
Validation logs are `artifacts\validation\pluginval-headless.txt` and
`artifacts\validation\pluginval-full.txt`; each rerun replaces its corresponding log.
Installations record `artifacts\validation\install-transcript.txt`; one-time setup
also records `artifacts\validation\install-setup-transcript.txt`.
For GUI validation without installation, use `scripts\Validate-Plugin.ps1` without
`-SkipGuiTests`. Its optional `-PluginPath` accepts another complete bundle path.

In **FL Studio 21.2.3 build 4004**:

1. Open **Options > File settings > Manage plugins**.
2. Ensure the standard `C:\Program Files\Common Files\VST3` path is enabled.
   Enable **Verify plugins** and click **Find installed plugins**. For updates,
   also enable **Rescan previously verified plugins**.
3. Confirm **LAZ-SYNTH** appears as a **64-bit VST3 instrument**, vendor
   **LAZ**, without an error. Star it to add it to favorites if desired.
4. Add it from **Channel Rack > + > More plugins**, or the Browser's
   **Plugin database > Installed > Generators > VST3**. The editor should show
   **Saw oscillator** and the MIDI/piano-roll prompt.
5. Close/reopen its editor, add a second instance, then save/reopen a test project.
   Both editors should reopen. Play MIDI and piano-roll melodies/chords; verify
   short/long/repeated/overlapping notes, looping, stop/restart, and audio export.
   Notes should stop after release without stuck voices or objectionable clicks.

FL Studio's documented VST3 scan locations are standard folders. The junction
provides a standard path while keeping the files on E:; adding E: or the staging
folder as a custom VST search path alone is insufficient. See the
[official installation guide](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm).
Record saw host results in STATE and the [M3 report](docs/validation/M3-fixed-saw-2026-10-12.md).
On 2026-10-12, the user confirmed the original scaffold loads and the renamed
LAZ-SYNTH installation on E: opens in FL Studio. Editor lifecycle, multiple
instances, and project save/reopen checks remain unreported.
Automated validation and actual FL Studio acceptance are separate checks.

## Resume work

Start Codex from this project directory. The root [AGENTS.md](AGENTS.md) directs
the agent to read two short files first:

1. [STATE](docs/STATE.md): current checkpoint and the next concrete action.
2. [CODE_MAP](docs/CODE_MAP.md): where the relevant information and code live.

Then use a prompt such as:

> Continue from docs/STATE.md. Verify the relevant files, complete the listed
> next action, run the applicable checks, and update the handoff before finishing.

For a different task, describe the desired result and constraints. The agent
should update the handoff to reflect the actual work performed.

## Project documents

| Document | Purpose |
| --- | --- |
| [AGENTS.md](AGENTS.md) | Stable instructions and the session workflow |
| [STATE](docs/STATE.md) | Current task, evidence, unfinished work, and next action |
| [CODE_MAP](docs/CODE_MAP.md) | Targeted navigation without routine full scans |
| [SPEC](docs/SPEC.md) | Requirements and unresolved product choices |
| [PLAN](docs/PLAN.md) | Milestone sequence and completion conditions |
| [DECISIONS](docs/DECISIONS.md) | Important choices and their rationale |
| [TESTING](docs/TESTING.md) | Validation procedure and evidence requirements |

STATE is the only current-status tracker. PLAN describes the route; DECISIONS
preserves reasoning. Documentation is maintained alongside the work, and disk
contents take precedence when a checkpoint is stale.

## Essential tools and libraries

The environment audit on 2026-10-11 found the following:

| Item | Version / configuration | Purpose and verification |
| --- | --- | --- |
| Visual Studio Community 2022 | 17.14.8; MSVC 19.44.35211, v143 toolset 14.44.35207 | C/C++ compiler and debugger; x64 Release compilation passed |
| Windows SDK | 10.0.26100.0 selected; 19041 and 22621 also installed | Windows headers, libraries, resources; runtime check passed |
| CMake / CTest | 3.31.6-msvc6, bundled with Visual Studio | Configure, build, and automated checks passed |
| Git for Windows | 2.45.1.windows.1 | Source control and dependency retrieval; local repository initialized on main |
| FL Studio x64 | 21.2.3.4004 | Original and renamed E: installations open, confirmed by the user |
| JUCE | 9.0.3, pinned in [dependencies.json](dependencies.json) | Installed; audio/DSP, MIDI, parameters, and GUI compiled and ran |
| pluginval | 1.0.4 | Installed; headless and full GUI scaffold validation pass |
| Windows curl | 8.21.0 | Official release downloads; setup uses curl.exe to avoid PowerShell's receive error |

Codex CLI 0.162.1 is available for development assistance; it is not a plugin
build dependency. Its version query reported access warnings for Codex's own
temporary directory. Those warnings did not affect the C++ build.

CMake is installed outside PATH at:

```text
C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
```

The scripts find this installation through Visual Studio Installer's `vswhere`;
no PATH edits or developer shell are needed. The project requires CMake >= 3.25
for its preset format. The presets select Visual Studio 2022, x64, and Windows
SDK 10.0.26100.0. All generated files stay inside this workspace.

On a new machine, install Visual Studio 2022 with **Desktop development with
C++**, including **MSVC v143 x64/x86 tools**, **C++ CMake tools for Windows**, and
**Windows SDK 10.0.26100.0**, plus Git for Windows. This machine already has them.

## Verified environment commands

Run from the project root in PowerShell:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Check-Environment.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset toolchain
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset juce
```

`Bypass` applies only to the launched PowerShell process. The audit distinguishes
missing JUCE from installed prerequisites; it does not replace compilation. The
second command configures, builds, and runs CTest for the offline toolchain check.
The toolchain preset passes one test; the JUCE preset passes both tests. The
toolchain check exercises Windows SDK calls, C++17, x64 targeting,
lock-free float atomics, and finite offline samples without opening an audio device.

Verified executable:
`build/toolchain/Release/ToolchainSmoke.exe`.
CTest evidence: `build/toolchain/Testing/Temporary/LastTest.log`.
JUCE check executable: `build/juce/JuceModuleSmoke_artefacts/Release/JuceModuleSmoke.exe`.
Both-test CTest evidence: `build/juce/Testing/Temporary/LastTest.log`.
Final source/artifact hashes and setup results:
[completed M1 report](docs/validation/M1-environment-complete-2026-10-11.md).
The [initial report](docs/validation/M1-2026-10-11.md) preserves the earlier,
partially complete checkpoint.

## Dependency setup

Dependencies are installed on this machine. To prepare another checkout, run
these verified commands from a network-enabled PowerShell at the project root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-Dependencies.ps1 -IncludePluginval
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset juce
```

Omit `-IncludePluginval` to download only JUCE. The setup script retrieves the
JUCE tag and checks its exact commit against `dependencies.json`. It preserves
existing dependency folders and stops on an unexpected revision or local edits.
JUCE goes in `third_party/JUCE`; pluginval goes in `tools/pluginval-1.0.4`.
Existing pluginval versions must match the lock. Windows curl downloads its
archive with redirects, a connection timeout, and checked error codes. Setup
does not change global PATH, execution policy, proxy settings, or Git trust.
Scripts resolve Windows path casing and trust only the JUCE directory for each
Git check, so the dependency also works across sandbox/user account ownership.
The pluginval release has no published checksum in its GitHub release metadata;
the script records the downloaded archive's SHA256, without claiming an upstream
checksum comparison or successful plugin validation.

The JUCE preset compiles a console integration check for its audio, DSP,
processor, and GUI modules. It checks oscillator/envelope output, release to
silence, MIDI/voice API use, a parameter, and a slider. It does not produce a
VST3 plugin or validate product polyphony, sound quality, or FL Studio integration.

## What JUCE provides for the synth

| Area | Selected building blocks |
| --- | --- |
| Base tones, filters, gain, modulation utilities | `juce_dsp`: oscillator, filters, oversampling, DSP processors |
| Voices, notes, amplitude envelope, smoothing | `juce_audio_basics`: voice/synth APIs, MIDI, ADSR attack/decay/sustain/release, smoothed values |
| FL Studio integration, automation, saved settings | `juce_audio_processors`: processor, parameters, parameter/state attachments; JUCE's bundled VST3 support |
| Basic editor and controls | `juce_gui_basics` and transitive GUI modules: sliders, buttons, labels, layout |
| Build/runtime checks | CTest and pluginval pass; opening in FL Studio confirmed by the user |

The JUCE version was checked against its [official release](https://github.com/juce-framework/JUCE/releases/tag/9.0.3)
and [CMake documentation](https://github.com/juce-framework/JUCE/blob/9.0.3/docs/CMake%20API.md).
This module selection covers the requested foundations; product oscillator design,
voice allocation, and UI behavior still need implementation and validation.
Separate DSP frameworks, FFT libraries, a standalone VST3 SDK download, Projucer,
Ninja, and web UI tooling are not prerequisites for this initial CMake setup.

JUCE is dual-licensed under AGPLv3 and its commercial licence; see the upstream
[licence file](https://github.com/juce-framework/JUCE/blob/9.0.3/LICENSE.md).
Private use is settled; a distribution licensing route is deferred. The setup
does not purchase a licence, create an account, or assign a release licence to
this project. Preserve upstream notices and revisit the route before distribution.

M2 supplied the VST3 foundation and build/validation/staging scripts above.
M3 adds the fixed saw for live MIDI and FL Studio's piano roll. Adopted behavior
and acceptance criteria are in [SPEC](docs/SPEC.md); FL listening and host acceptance
must be recorded separately from automated validation. Sustain, pitch bend, and
modulation are deferred. When all eight voices are busy, an old voice fades for
at most 1 ms before its replacement starts.

## Working together

Codex maintains the implementation, checks, and handoff. The user supplies sound
and interface direction and performs listening and host checks in FL Studio.
Record the build, preset, MIDI steps, sample rate, and buffer size when reporting
an audio problem so it can be reproduced in a later session.
