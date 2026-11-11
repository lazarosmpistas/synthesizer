# M4 amplitude envelope validation - 2026-10-21

Checkpoint: uncommitted ADSR changes based on source commit `918b179`.
Existing documentation edits recording M3 listening acceptance were preserved.
Windows x64 Release, MSVC v143, pinned JUCE 9.0.3, LAZ-SYNTH v0.1.0 by LAZ.
The exact source and binary hashes below identify this validation checkpoint.

## Behavior delivered

- Four automatable amplitude controls: attack/decay 1–5000 ms, sustain 0–100%,
  release 1–10000 ms. Defaults 3/100 ms, 100%, 5 ms preserve the original saw.
- Linear per-voice ADSR; early note-off releases the current level. Active
  duration/decay-target changes retarget continuously; sustain changes ramp over
  10 ms. Parameter reads occur once per callback. Existing MIDI/voice policies,
  bounded 1 ms stealing, tuning, velocity, and gain remain.
- Version 1 parameter state, legacy empty-state defaults, atomic rejection of
  invalid/incomplete/unknown-version state. No live notes are saved.
- Native 640x400 editor with rotary controls, typed values/units, double-click
  reset, and two-way host attachments. Reported maximum tail: 10 seconds.

## Commands and outcomes

Executed from `C:\Users\lazar\PycharmProjects\synthesizer`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Checks.ps1 -Preset plugin
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Validate-Plugin.ps1 -SkipGuiTests
& .\build\plugin\PluginScaffoldCheck_artefacts\Release\PluginScaffoldCheck.exe --editor-preview 'C:\Users\lazar\PycharmProjects\synthesizer\artifacts\validation\adsr-editor-final.png'
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-Plugin.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-Plugin.ps1 -Install -Validate
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\Validate-Plugin.ps1 -SkipGuiTests -PluginPath 'C:\Program Files\Common Files\VST3\LAZ-SYNTH.vst3'
git -c safe.directory=C:/Users/lazar/PycharmProjects/synthesizer diff --check
```

- PASS: final configure/build and CTest 2/2 in 0.24 s, including the real product
  processor/editor. Regression checks still cover saw pitch/harmonics/alias
  reduction, all MIDI pitches, sample-position MIDI, FIFO/overlap/channels/panic,
  voice exhaustion/queued stealing, boundaries/empty blocks, lifecycle, and
  changing block sizes at 44.1/48/96 kHz.
- PASS: ADSR timing and live retargeting at all three rates, early attack/decay
  note-off, zero sustain, actual audio from all four parameters, extremes and
  10-second release, settings/sound round trip, legacy empty state, malformed,
  incomplete, non-finite, out-of-range and unsupported-version state rejection.
- PASS: editor recreation, two-way parameter/knob updates, full offscreen
  software painting and visual inspection of the PNG above. Labels, units, knobs,
  and editable values are visible and fit within the editor.
- PASS: actual VST3 headless pluginval 1.0.4 at strictness 5, seed 12345,
  44.1/48/96 kHz, 64/256/1024 samples, including state and automation tests.
- PASS: workspace staging; every copied bundle file matches the build SHA256.
- PASS: approved physical E: install with non-administrator token, full GUI
  pluginval including editor/state/automation/editor-automation checks, and
  headless native loading through the existing C: standard-folder junction.
- PASS: source review of audio paths: fixed arrays, non-owning MIDI, cached float
  atomics; no allocation, blocking locks, file access, or logs in the callback.
- PASS: Git whitespace check and ignore review. Existing exclusions cover new
  build/validation/PNG outputs; tracked files contain no generated artifacts.
- NOT RUN: standalone Steinberg validator (not installed; pluginval skips it).
- NOT RUN: performance benchmark (no target/workload agreed); no optimization
  or benchmark result is claimed.

Resolved development failures: the first harness run found an extra attack sample
at 48 kHz because APVTS's initial skew-converted default was slightly off-grid.
Time values now snap to the advertised 0.1 ms steps before sample conversion.
A later editor check caught unreadied offscreen image output; the harness now
uses software images and ends drawing before checking/encoding pixels. The final
build/CTest above passes after both fixes.

Local logs: `build/plugin/Testing/Temporary/LastTest.log`,
`artifacts/validation/pluginval-headless.txt` (latest run: C: junction),
`artifacts/validation/pluginval-full.txt` (E: full GUI), and
`artifacts/validation/install-transcript.txt`. These are generated and replaceable.

## Installation checkpoint

Built/staged bundle: `LAZ-SYNTH.vst3` under
`build/plugin/Synthesizer_artefacts/Release/VST3` and `artifacts/VST3`.
Installed bundle: `E:\everything_music_related\VstPlugins\LAZ-SYNTH.vst3`.
FL scan entry: `C:\Program Files\Common Files\VST3\LAZ-SYNTH.vst3`, the existing
junction to E:. Both were validated after the update. The previous M3 install is
preserved at `artifacts/install-backups/2494cc4164c845078bf58811225135ae`.
Readback of binary and manifest SHA256 matches across build/stage/E:/junction;
the following two hashes apply to all four copies. No commit/push was performed.

Stable component CID: `ABCDEF019182FAEB4C617A6153796261`.
Stable controller CID: `ABCDEF011234ABCD4C617A6153796261`.
Build manifest name/vendor/version: LAZ-SYNTH / LAZ / 0.1.0.

| Bundle file (build/stage/install/junction) | SHA256 |
| --- | --- |
| `Contents/x86_64-win/LAZ-SYNTH.vst3` | `D389C9FFB99EDA038FCF2B66469BE4F4AFCA680D91A2CF8B309053755235D228` |
| `Contents/Resources/moduleinfo.json` | `E9A6FA337DEB5538894374204C5DD5A706D5E7A224DAA7D004AACC2308C5C005` |

## Validated source hashes

| Path | SHA256 |
| --- | --- |
| `CMakeLists.txt` | `3FCB17F857AB10BDF8E3E2F9CAA52A7A7F5CE26A9C6575F8DA4C172CC3D4918B` |
| `Source/DSP/AmpEnvelope.h` | `36498F19243BD9451A331BDAF7EB81C60D4EE004F012F8BE47027584C113A161` |
| `Source/DSP/SawOscillator.h` | `4F2D66C32C7E1FE84C9F6EBC49EFCA0213E95879DCC2D41189B52E9B2E66BBC9` |
| `Source/DSP/SawVoiceEngine.h` | `C786CC5E3861E14C6F79ABAF9F415606A2CAEB9269143A756B27B1E7B0353368` |
| `Source/DSP/SawVoiceEngine.cpp` | `37D0C0E26B3B9603C85DB9D6FF5B7C23DF29428165B315BA9763D91C40C654E1` |
| `Source/Plugin/Parameters.h` | `53D5795948D8066C7051AC3D44763A389F63F59E92EF9F80B4B875A02603B491` |
| `Source/Plugin/PluginProcessor.h` | `CC2DC68E56FFA36A39E3F1CB36769F5A80BABFE71195C28EA8637DF6D4957E20` |
| `Source/Plugin/PluginProcessor.cpp` | `05D8BD917AB4242F0AFEA0609AAA88839BDF8E109A0D09AF785BAE7CE7DCD176` |
| `Source/UI/PluginEditor.h` | `7452550A356EAC4EAB20693230822322CDF38ADEE193B54F86E59250A9240768` |
| `Source/UI/PluginEditor.cpp` | `54103E2E22ECB9C701BC55606C77B80DE367282559C8A6396ED1CA6B2A1AE5F0` |
| `tests/PluginScaffoldCheck.cpp` | `EF1AE4EC08B7E80EBB670585DF18807BA65EC9AE8FCD4EF986BAE7DE5BCECBAD` |

## Pending FL Studio acceptance

NOT RUN by the agent / UNREPORTED by the user for this ADSR artifact:

1. Record FL version, sample rate, and buffer. Play live MIDI and piano-roll
   notes/chords, short/long/repeated/overlapping notes, loops, and stop/restart.
2. Hear attack clearly at about 500 ms; lower sustain to about 30% and vary decay;
   verify sustain while held and release after note-off, including early note-off.
3. Automate all four controls during held/releasing notes; listen for artifacts.
4. Export a long release tail; check editor reopening and independent instances.
5. Save/reopen a project with changed ADSR settings; verify values and sound.
   Open an older fixed-saw project and confirm the original default sound.

The user's M3 listening acceptance applies to the earlier artifact only; see
[M3 report](M3-fixed-saw-2026-10-12.md). Its unreported host checks remain open.
