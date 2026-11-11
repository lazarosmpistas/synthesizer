# Current state and session handoff

Updated: 2026-11-12. Replaceable checkpoint; verify disk before continuing.

## Current task

- User requested committing/pushing the latest ADSR work, excluding sensitive
  and irrelevant files. Reviewed source/docs and expanded local/credential ignores.
- M4's first addition is a four-control amplitude ADSR with automation and saved
  settings (SPEC/D013).
- Implemented, built, validated, and installed. Final x64 Release/CTest 2/2,
  headless/full GUI pluginval, staging/E: install/junction loading all pass.
- FL listening, automation, project recall, and old-project checks are pending.
- Target: Windows x64, FL Studio 21.2.3 build 4004; private development.

## Last completed checkpoint

- Added per-voice linear ADSR and four native knobs (640x400 editor), editable
  values, reset gestures, host parameters, versioned state, legacy empty-state
  defaults, and invalid-state rejection. Stable VST3 identity/version retained.
- Adopted ranges: attack/decay 1-5000 ms, sustain 0-100%, release 1-10000 ms.
  Defaults 3/100 ms, 100%, 5 ms retain the old sound. Callback snapshots atomics;
  active stages retarget continuously, sustain changes ramp for 10 ms. Stealing
  stays at most 1 ms. Conservative reported tail is 10 seconds.
- Product harness covers ADSR timing at three rates, live retargeting, zero
  sustain/early note-offs, actual audio/parameter wiring, limits/long release,
  saved sound/state, invalid/legacy state, and two-way controls/editor recreation.
- Existing oscillator/MIDI/polyphony regressions pass. Final editor PNG inspected.

## Work in progress

- No unfinished source/build/install work. FL host/listening acceptance remains.
- ADSR source/docs prepared for commit/push on main above 918b179; use Git
  log/status and origin/main to verify the outcome. Earlier M3 documentation
  edits remain, including accepted listening and outstanding host checks.
- Physical installed plugin is now the ADSR build at
  E:/everything_music_related/VstPlugins/LAZ-SYNTH.vst3, with existing C: junction.
- Prior M3 install backed up inside artifacts/install-backups/
  2494cc4164c845078bf58811225135ae. Updates used approved non-admin execution.
- Build/artifacts/dependencies/install config remain ignored; local agent/cloud
  settings, environment secrets, and signing keys are now explicitly ignored.

## Validation at this checkpoint

- PASS (2026-11-12): powershell -NoProfile -ExecutionPolicy Bypass -File
  .\scripts\Build-Checks.ps1 -Preset plugin; x64 Release, CTest 2/2, 2.25 s.
- PASS (2026-11-12): tracked/proposed-file credential-pattern scan; no matches.
  Tracked-file review found only source, tests, configuration, and project docs.
- Results below are historical 2026-10-21 evidence; pluginval/install/FL checks
  were not repeated for this commit task. Product source remains unchanged.
- PASS: powershell -NoProfile -ExecutionPolicy Bypass -File
  .\scripts\Build-Checks.ps1 -Preset plugin; final CTest 2/2, 0.24 s.
- PASS: Validate-Plugin.ps1 -SkipGuiTests; strictness 5, seed 12345,
  44.1/48/96 kHz, 64/256/1024 samples, actual ADSR state/automation.
- PASS: Stage-Plugin.ps1; all staged files match build SHA256.
- PASS: optional harness --editor-preview output visually inspected at
  artifacts/validation/adsr-editor-final.png. Software drawing includes controls.
- Resolved during development: off-grid APVTS defaults added one attack sample;
  quantizing to advertised 0.1 ms steps preserves default fades. Offscreen image
  drawing now completes on a software image before it is read/encoded.
- Source audit: fixed arrays/atomic reads/non-owning MIDI; no allocation, blocking
  locks, files, or logs in the product callback. No performance claim.
- PASS: Stage-Plugin.ps1 -Install -Validate; full GUI/state/automation/editor
  automation checks pass at E:. Existing C: junction loads natively in pluginval.
- PASS: build/stage/E:/junction binary and manifest SHA256 match. Binary:
  D389C9FFB99EDA038FCF2B66469BE4F4AFCA680D91A2CF8B309053755235D228.
- PASS: Git diff --check, ignore/tracked-file review, 71 local doc links resolve.
- Source/artifact hashes and exact commands: [M4 report](validation/M4-amp-envelope-2026-10-21.md).
- UNREPORTED: FL ADSR listening, automation, saved settings/older-project loading,
  release export tails, editor reopen, and independent instances.
- Historical M3 user report (2026-10-21): notes/chords/no clicks/overall playback
  accepted; loops/stop/export/two instances/editor/save-reopen remain unreported.
- Standalone Steinberg validator is unavailable; pluginval skips it.

## Next action

Start with docs/validation/M4-amp-envelope-2026-10-21.md's FL checklist and
docs/TESTING.md's M4 guidance. Reopen LAZ-SYNTH, listen to each knob (lower sustain
to hear decay), automate controls, verify long-release export, editor recreation,
two instances, project save/reopen, and an older fixed-saw project. Record host
version/rate/buffer. Completion: required checks pass, or reproduce/fix a failure
and rerun affected validation. Do not infer host acceptance from pluginval.

## Open questions and blockers

- No build/install blocker. E: must remain available for its C: junction.
- No reported sound issue; ADSR FL host/listening acceptance remains pending.
- MIDI sustain pedal/pitch bend, filters/effects, and broader sound design remain
  later choices. Performance target unspecified; settle licensing before release.
