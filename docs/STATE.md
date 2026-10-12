# Current state and session handoff

Updated: 2026-10-12. Replaceable checkpoint; verify disk before continuing.

## Current task

- User requested the basic saw oscillator build. M3 is implemented, built,
  validated, and installed; FL Studio listening/host acceptance is pending.
- Selected scope: MIDI/piano-roll saw, no effects, filters, or amp controls.
- Adopted defaults: eight PolyBLEP voices, A4=440 Hz, velocity, gain 0.1/voice,
  fixed 3/5 ms fades and at most 1 ms fade before a stolen slot starts its note.
  FIFO surviving-note release/channels/panic/queued stealing are in SPEC/D012.
- Target host: FL Studio 21.2.3 build 4004, Windows x64; private development.
- Earlier user AGENTS.md tracking-hygiene changes are preserved and uncommitted.

## Last completed checkpoint

- Final M3 x64 Release build/CTest 2/2, headless/full GUI pluginval, staging,
  E: installation, and native loading through the existing C: junction passed.
- Implemented Source/DSP saw/voice engine, processor sample-position MIDI and
  lifecycle reset, fixed 5 ms reported tail, and editor's playable-saw status.
- Product harness passed pitch/harmonics/alias reduction, fades/velocity, all
  128 notes at 44.1/48/96 kHz, timing/block equivalence, overlap/channels/stealing,
  dense bursts/panic, lifecycle/state/layout/editor, FIFO, empty/boundary blocks,
  and long-held notes. Exact source/binary hashes are in the M3 report below.
- Plugin remains LAZ-SYNTH v0.1.0 by LAZ, JUCE 9.0.3, MSVC v143/CMake.
- Stable identity Laza/Syba and D007 CIDs are retained; no user parameters/settings.
- Historical M2 scaffold opened in FL (user report); this does not validate M3.

## Work in progress

- No unfinished source work. Installed binary is now the playable M3 saw.
- M3 milestone remains open until the user confirms FL listening/host behavior.
- Build/stage are inside the workspace. Physical install is
  E:/everything_music_related/VstPlugins/LAZ-SYNTH.vst3; standard C: VST3 entry
  is its existing junction. Update used a non-administrator token with sandbox
  approval; the prior M2 installation was backed up inside artifacts/install-backups.
- Main starts at 976223f; no commit/push performed this turn. Generated outputs
  and dependencies remain ignored. plugin-install.json is already ignored and
  untracked on disk (the prior handoff's tracking warning was stale).

## Validation at this checkpoint

- PASS: Build-Checks.ps1 -Preset plugin; x64 Release and final CTest 2/2, 1.08 seconds.
- PASS: Validate-Plugin.ps1 -SkipGuiTests; built M3 VST3, strictness 5, seed 12345,
  44.1/48/96 kHz, 64/256/1024 samples. Steinberg standalone validator unavailable.
- PASS: coherent 9 kHz check folded-bin power is 0.0420173 times naive saw power;
  this is a measured case, not a claim of alias-free output or performance.
- Source audit: rendering uses fixed arrays and non-owning MIDI bytes; no
  allocation, blocking locks, file access, or logs in product callback paths.
- PASS: Stage-Plugin.ps1; Stage-Plugin.ps1 -Install -Validate (E: full GUI);
  Validate-Plugin.ps1 -SkipGuiTests -PluginPath for the existing C: junction.
- PASS: build/stage/E:/junction binary+manifest hashes match; branding/stable CIDs,
  Git diff --check, ignore rules, and document links reviewed at the final source.
- Evidence/commands/source hashes: [M3 report](validation/M3-fixed-saw-2026-10-12.md).
- NOT RUN: M3 FL Studio listening/live MIDI/piano roll/export/lifecycle/project
  checks; requires user participation. No performance benchmark claimed.
- Prior M2/M1 evidence remains in docs/validation; it validates historical artifacts.

## Next action

Start with docs/validation/M3-fixed-saw-2026-10-12.md's pending FL checklist.
User loads a fresh installed LAZ-SYNTH instance (editor says Saw oscillator),
checks live MIDI/piano-roll chords, loops, stop/restart/export, two instances,
and project save/reopen. Record sample rate/buffer and any issue in this checkpoint
and the M3 report. Read TESTING M3/README as needed. Completion: user reports clean
sound/no stuck notes and passing host checks, or a reproducible issue is fixed
and the affected source/artifact checks are rerun before another listening pass.

## Open questions and blockers

- No build/install blocker; E: must remain available for the junction.
- FL musical/listening acceptance is pending; fixed fades may need adjustment.
- Sustain, pitch bend, modulation, broader sound design, UI, and performance target
  remain later choices. Preserve JUCE notices; settle licensing before distribution.
