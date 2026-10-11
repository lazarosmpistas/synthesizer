# Milestone plan

This file describes deliverables and their completion conditions. Use
[STATE](STATE.md) for current status and the next action; do not maintain a second
live task tracker here. Milestones may be refined as product choices settle.

## M0 - Documents and session continuity

Deliverables: Agent instructions, current-state handoff, navigation map, product
specification, milestone plan, decisions, testing guidance, and a README.

Complete when relative links resolve, existing requirements are preserved, status
claims match the workspace, and a new session has a concrete starting point.

## M1 - Development environment

Deliverables: Evidence-based audit of the Windows compiler, SDK, CMake, Git, and
Codex; FL Studio host details; pinned JUCE source and documented licensing status;
Git repository and ignore rules; a minimal C++ compilation check; a JUCE audio,
DSP, MIDI/voice, parameter, and GUI module compilation/runtime check. Optional
pluginval setup prepares M2 validation.

Complete when both environment checks compile and run, the pinned JUCE checkout
is present, and README records the actual setup and licensing status. Identify
any installation or network-enabled setup steps the user needs to perform.

## M2 - Plugin scaffold and verification commands

Deliverables: Extend the M1 pinned dependencies and CMake setup with a minimal
VST3 instrument target, a lightweight validation harness, and configure/build/test/
validate/stage operations. Update CODE_MAP with actual entry points and artifacts.

Complete when the artifact builds, the configured checks pass, and FL Studio can
scan and load it using the documented procedure. Sound generation belongs to M3.

## M3 - First playable instrument

Selected scope: A fixed saw sound playable with MIDI and ordinary FL Studio
piano-roll notes, with no effects, filters, or adjustable amplitude settings.
Voice count, velocity response, and short internal fades have recommended defaults
in SPEC; record the adopted behavior when implementation begins.

Deliver in this order:

1. Add the saw oscillator and a preallocated voice engine under Source/DSP.
   Reduce aliasing, establish tuning and fixed output headroom, and keep allocation
   and blocking locks out of rendering. Short fixed note fades are recommended
   to reduce clicks, with no user controls.
2. Connect MIDI note-on/note-off to the engine at their sample positions in
   PluginProcessor. Define repeated/overlapping notes, voice stealing, all-notes-off,
   all-sound-off, and lifecycle reset behavior. Retain stereo output and identity.
3. Adapt the product harness: silence is expected while idle and after release,
   while held notes must produce pitched audio. Check pitch, MIDI timing, finite
   bounded output, chords, repeated notes, voice exhaustion, and rate/block changes.
   Keep existing layout/editor/lifecycle coverage and update the editor's status text.
4. Run the documented build/CTest and pluginval commands, then stage/install and
   validate the exact built bundle using the existing workflow.
5. Record FL Studio live-MIDI and piano-roll listening checks, including looping,
   stop/restart, audio export, multiple instances, and project save/reopen.

Complete when automated behavior checks and artifact validation pass, and the
user confirms the installed saw plays correctly in FL Studio without stuck notes
or objectionable clicks. Automation and settings controls belong to a later
feature milestone; saving/reopening an FL project remains an M3 host check.

## M4 - Sound development and interface refinement

Deliverables: Small, individually validated additions selected from the product
specification. Possible additions include filters, oscillators, modulation,
polyphony, effects, presets, and interface improvements; none is committed yet.

Complete each addition with behavioral checks, relevant host/listening checks,
documentation updates, and a recoverable source checkpoint.

## M5 - Performance, compatibility, and packaging

Deliverables: Defined performance measurements, expanded compatibility checks,
repeatable clean builds, versioned artifacts, and a documented installation or
distribution procedure appropriate to the user's goals.

Complete when the agreed matrix passes and remaining limitations are recorded.
