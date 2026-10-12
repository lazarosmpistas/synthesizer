# Validation guide

This document defines checks and evidence. Actual results belong in
[STATE](STATE.md). Executable commands belong in [README](../README.md) once
implemented and verified. Environment and plugin-scaffold checks now exist.

## Documentation changes

- Check that referenced local files and relative Markdown links resolve.
- Verify that CODE_MAP names real files and labels proposed paths explicitly.
- Check that STATE gives an accurate checkpoint and concrete next action.
- Keep current progress in STATE; avoid conflicting status trackers.
- Confirm that requirements, recommendations, and unresolved choices are distinct.
- No audio build or runtime tests are required for documentation-only changes.

## Environment checks

The commands and their verified/pending status are in [README](../README.md).

- `scripts/Check-Environment.ps1` audits installed tools, the selected SDK, host
  executable version, and dependency revision. Missing JUCE is reported; this
  read-only audit is not a compilation result.
- `scripts/Build-Checks.ps1 -Preset toolchain` configures/builds x64 Release and
  runs `environment.toolchain`. It verifies Windows SDK/runtime linking, C++17,
  lock-free float atomics, and finite offline samples. It requires no JUCE or
  audio device. The compiler treats source warnings as errors.
- `scripts/Build-Checks.ps1 -Preset juce` requires the actual locked, clean JUCE
  checkout. It additionally compiles/links GUI, audio, processor, and DSP modules
  and runs `environment.juce-modules`.
  It tests basic module use, not product voice handling or parameter automation.
- A missing or unexpected JUCE checkout must stop configure. A failed native
  command must propagate a nonzero script result and stop later steps.
- Check PowerShell syntax and JSON parsing after setup-script or preset changes.
  Inspect Git ignore rules so generated artifacts and third-party code stay out
  of source control. Preserve existing dependency edits on setup failures.
- Setup and audit must reject an installed pluginval version that differs from
  dependencies.json. Run its --version/--help checks after first installation;
  this establishes tool availability, not successful plugin validation.
- Use Windows canonical path casing for generated builds and per-command Git
  ownership exceptions; global safe.directory or PATH changes are unnecessary.

Evidence: [completed M1 report](validation/M1-environment-complete-2026-10-11.md)
and the historical [initial report](validation/M1-2026-10-11.md).
Executables are `build/toolchain/Release/ToolchainSmoke.exe` and
`build/juce/JuceModuleSmoke_artefacts/Release/JuceModuleSmoke.exe`. Local CTest
logs are under each build tree's `Testing/Temporary/`.

## M2 plugin scaffold (historical behavior; M3 extends the same commands)

- `Build-Checks.ps1 -Preset plugin` builds the actual x64 Release VST3 and runs
  `environment.toolchain` plus `plugin.scaffold`. The JUCE smoke preset remains
  separate; the plugin check links the same product processor/editor as the VST3.
- `plugin.scaffold` checks no audio input, stereo output, rejection of mono,
  surround, and audio-input layouts; exact silent/finite output from dirty
  buffers at 44.1/48/96 kHz and 0/1/64/256/1024 samples with MIDI; repeated
  prepare/release; and editor recreation/offscreen painting. It opens no native
  editor window and requires no audio device. This does not prove FL integration.
- `Validate-Plugin.ps1 -SkipGuiTests` runs the pinned pluginval on the built
  bundle at strictness 5, seed 12345, 44.1/48/96 kHz and 64/256/1024 samples.
  Without this switch it includes GUI open/close/recreation tests. The Windows
  GUI executable must be invoked through a pipeline so PowerShell waits and
  propagates its exit code; a displayed SUCCESS alone is insufficient.
- `Stage-Plugin.ps1` copies the entire bundle to `artifacts/VST3` and compares
  each source/copied file's SHA256. Repeated staging must pass. `-Install` uses
  the directory in plugin-install.json (currently E:\everything_music_related\VstPlugins);
  `-Validate` then validates the copied
  artifact, including GUI tests. Existing matching installations are backed up;
  differing class IDs stop replacement. Read moduleinfo class IDs without
  strict JSON parsing: the SDK manifest contains trailing commas.
  Physical updates do not request UAC and record artifacts/validation/install-transcript.txt.
  After the LAZ-SYNTH rename, verify name/vendor in the actual moduleinfo and
  unchanged component/controller CIDs. The binary name follows the bundle name.
  After copy/validation, matching legacy Synthesizer bundles are moved to
  same-volume backups; both absolute paths are checked before the move.
- `Configure-PluginInstall.ps1 -Preview` verifies/prints the one-time storage and
  standard-folder junction paths without mutation. Without Preview it relocates
  the original bytes to E: first, installs/validates the renamed E: bundle,
  creates a per-plugin standard-folder junction, validates native loading through
  it, and backs up the original C: bundle. UAC is requested only if the junction
  or old-install migration needs administrator rights. It preserves unrelated
  standard-folder entries. Test the link target, source/E:/junction hashes and
  unchanged CIDs; then verify a physical E: update works with a non-admin token.
- The scaffold deliberately outputs silence, consumes MIDI, has no parameters,
  and stores no user state. Sound quality, automation, restored settings, and
  performance acceptance remain future work. No Steinberg standalone validator
  is installed; pluginval explicitly skips that optional test.

Evidence belongs in [M2 report](validation/M2-plugin-scaffold-2026-10-12.md).
Raw CTest evidence: `build/plugin/Testing/Temporary/LastTest.log`.
Raw pluginval evidence: `artifacts/validation/pluginval-headless.txt` or
`pluginval-full.txt`. Identify the checked binary/manifest by hashes.

## M3 fixed saw

- `plugin.scaffold` is the retained CTest name for the product harness. It now
  checks pitched held notes and idle/released silence, retaining layouts,
  dirty-buffer clearing, editor recreation, and prepare/release coverage.
- Verify pitch (including MIDI note 69 at 440 Hz if recommended tuning is adopted),
  saw harmonic content, and finite/bounded output across all 128 MIDI notes.
  The coherent 9 kHz oscillator check compares folded-bin power at 3/12/21 kHz
  to an uncorrected saw. This proves reduction in that case, not ideal bandlimiting.
- Place note-on/off events inside blocks and at boundaries; verify that timing
  follows event sample positions and that rendering is consistent across block sizes.
- Check note-off and zero-velocity note-on, repeated and overlapping pitches,
  separate MIDI channels, chords, and more notes than the adopted voice limit.
  Verify the defined voice-stealing behavior and eventual release to silence.
- Verify all-notes-off, all-sound-off, prepare/release/reset, finite samples,
  and fixed output bounds at maximum velocity and maximum simultaneous voices.
- Exercise 44.1/48/96 kHz, 0/1/64/256/1024 samples, long held notes, and changing
  block sizes. Audit rendering for allocation, blocking locks, file access, and logs.
- If velocity response and fixed fades are adopted, check velocity-dependent
  loudness and fade duration; listen to starts, releases, and stolen voices for clicks.
  Check FIFO repeated-note release, releasing-slot preference, oldest-held stealing,
  queued-note cancellation, and dense bursts. Replacement onset may be delayed
  by at most the adopted 1 ms stealing fade when every slot is busy.
- Run the documented plugin build/CTest, pluginval, and stage/install validation.
  Record source and artifact hashes; earlier scaffold checks do not validate M3.
- In FL Studio 21.2.3 build 4004, play live MIDI and a piano-roll melody/chord
  pattern. Check short notes, long notes, repeated notes, overlapping notes, loops,
  stop/restart, audio export, two instances, editor reopening, and project save/reopen.
- Record ordinary piano-roll note playback separately from any optional MIDI
  expression. Automation tests are not applicable while there are no user parameters.

## Code and build changes

Run checks appropriate to the affected behavior using the documented commands.
Re-run affected validation after fixes or subsequent changes; broaden testing
when failures or unresolved concerns justify it.

| Area | Useful checks |
| --- | --- |
| Build/configuration | Compilation and a fresh configure/build after dependency or toolchain changes |
| Oscillator/envelope | Pitch, envelope timing, finite output, release to silence |
| MIDI/voices | Event timing, repeated notes, sustain, voice stealing, no stuck notes |
| Parameters/state | Automation, smoothing, save/restore, supported state compatibility |
| Plugin lifecycle | Validation tool, editor reopening, multiple instances, buffer/rate changes |
| Performance | Release-build timing with a specified preset, voice count, rate, and buffer |

Choose tests that exercise expected behavior and meaningful edge cases. Do not
add tests solely to reproduce the implementation's internal structure.

## FL Studio checks

1. Stage/install the exact built artifact using the documented procedure.
2. Scan and load it as an instrument; record the FL Studio version.
3. For M2, verify the silent-build editor, editor reopening, two instances, and
   project save/reopen. Sound/automation checks below begin in M3.
4. Play notes and chords appropriate to the supported voice mode; test release,
   repeated notes, and sustain when supported.
5. Exercise relevant controls and automation while listening for artifacts.
6. Save/reopen a project and confirm parameters and the sound are restored.
7. Check multiple instances and closing/reopening the editor.
8. Exercise supported sample rates and buffers; initial proposed cases are
   44.1/48/96 kHz and 64/256/1024 samples, subject to the actual setup.
9. Record remaining problems with preset, MIDI sequence, and reproduction steps.

Project save/reopen is required for the first playable milestone. Automation and
settings-restoration checks become required when user parameters are introduced;
M3's fixed saw has no such controls. Additional checks depend on supported features.

## Recording evidence

For each check, record PASS, FAIL, or NOT RUN with a reason. Include the exact
command or manual steps, date, source revision or changed-file checkpoint, build
configuration, artifact path, and relevant environment. Capture useful errors
without dumping credentials or excessive logs into the handoff.

For long evidence, keep a separate report and link it from STATE. A prior passing
check applies to the checkpoint it tested. A passing build does not establish
musical quality or FL Studio compatibility; record those checks separately.
