# Product specification

This file holds requirements and open product choices. Track current progress in
[STATE](STATE.md) and milestone sequencing in [PLAN](PLAN.md).

## Established direction

- Deliver a synthesizer instrument usable in FL Studio.
- Target FL Studio 21.2.3 build 4004 (x64), confirmed on disk on 2026-10-11.
- Target Windows x64 and the VST3 plugin format.
- Use C++, JUCE, and CMake, as specified in the original AGENTS.md.
- Use C++17 for the initial environment; JUCE 9.0.3 is the pinned framework.
- Develop for private use now; decide distribution and its licensing later.
- Maintain concise project documentation so work can continue across sessions.
- Make implementation changes reviewable through repeatable builds and checks.

## M2 foundation acceptance

- A Windows x64 VST3 instrument that FL Studio can discover and open.
- Native editor, MIDI input, no audio input, and stereo output.
- Silent processing until sound-engine choices are settled for M3.
- Repeatable build, automated validation, staging, and standard-folder installation.
- Plugin name: LAZ-SYNTH; vendor: LAZ, explicitly selected by the user on
  2026-10-12 after confirming the original scaffold loads fine in FL Studio.
  Keep the original VST3 identity codes stable (see DECISIONS D007/D009).
- Physical installation: E:\everything_music_related\VstPlugins, selected by
  the user to avoid recurring Program Files write issues. A per-plugin junction
  provides FL Studio's standard VST3 scan path (DECISIONS D010).

## M3 first playable milestone: fixed saw

Scope selected by the user on 2026-10-12 during planning:

- A simple saw sound playable through MIDI and ordinary FL Studio piano-roll notes.
- Notes sound while held and stop after note-off.
- No effects, filters, or user-adjustable amplitude settings in this milestone.
- Keep the existing Windows x64 VST3 identity and installation workflow.

Implementation defaults adopted on 2026-10-12 for the requested basic saw
(engineering choices within the selected scope):

- Eight simultaneous voices, one saw oscillator per voice, for melodies and chords.
- Standard equal-tempered tuning with MIDI note 69 at 440 Hz.
- MIDI note velocity controls loudness; no additional gain control in the editor.
- A PolyBLEP saw to reduce discontinuity aliasing; residual aliasing remains.
  Fixed gain is 0.1 per voice, with a worst-case eight-voice peak below 0.8.
- Short linear fades: 3 ms attack and 5 ms release, rounded up to whole samples.
  Check by listening. These are internal constants,
  with no ADSR controls or sound-shaping envelope in the interface.
- A preallocated voice pool and sample-position MIDI processing, with no allocation
  or blocking locks in the audio callback. Repeated pitches create separate voices;
  note-off releases the oldest still-held/queued match on that MIDI channel.
- When full, prefer an already-releasing voice, then the oldest held voice.
  Fade the old voice for at most 1 ms before starting its replacement. If every
  slot is fading, replace the oldest queued note without extending its fade.
  A note-off can cancel a queued note. Stolen notes are discarded, not restored.
- MIDI CC123 releases that channel's held voices and cancels its queued notes;
  CC120 immediately silences that channel. Prepare, release, and reset clear voices.
- Leave sustain, pitch bend, modulation, and presets for later feature choices.

Acceptance: pitched saw output follows note timing, becomes silent after release,
stays finite and within the chosen output bounds, and handles repeated/overlapping
notes without stuck voices. Verify at 44.1/48/96 kHz and varying buffer sizes.
In FL Studio, verify live MIDI, piano-roll melodies/chords, looping, stop/restart,
audio export, two instances, and project save/reopen against the installed artifact.

M3 deferred parameters/automation/state until controls were introduced in M4.
M3 still checks project save/reopen with the fixed sound;
active notes are transient and must not be restored as held voices. The broader
subtractive-synth direction remains open beyond this first oscillator.

## M4 first addition: amplitude envelope controls

User request on 2026-10-21: basic knobs, including attack, release, and decay.
Implement attack, decay, sustain, and release for each voice's amplitude envelope.
Filters, effects, extra oscillators, and MIDI expression remain later choices.

Adopted implementation choices within that request:

- Four automatable rotary knobs with editable values and double-click reset.
- Attack/decay: 1–5000 ms; release: 1–10000 ms, in 0.1 ms steps. Time knobs
  favour short values (midpoints 250/250/500 ms). Sustain: 0–100% in 0.1% steps.
- Defaults: attack 3 ms, decay 100 ms, sustain 100%, release 5 ms. These preserve
  the original saw's sound; decay becomes audible when sustain is below 100%.
- Linear ADSR stages. Note-off during attack/decay releases from the current level.
  Live duration/decay-target changes retarget an active stage from its current
  level over the new duration. Sustain-stage changes ramp over 10 ms. Parameters
  are read once per callback. No sample-accurate parameter automation is claimed.
- Retain eight voices, velocity, tuning, gain, FIFO release, channel panic, and
  at most 1 ms stealing fades. Parameter changes cannot extend a stealing fade.
- Stable IDs `ampAttackMs`, `ampDecayMs`, `ampSustain`, `ampReleaseMs`; version 1
  state contains parameters only. Legacy empty M2/M3 state loads the original
  defaults. Invalid, incomplete, and unsupported-version state is ignored.
- Report the maximum 10-second release as the tail to cover settings changes.
  Keep the existing VST3 identity and installation workflow.

Acceptance: each knob audibly controls its ADSR stage, including short note-offs
and zero sustain; changes stay continuous and bounded; voices release without
becoming stuck. Verify 44.1/48/96 kHz, parameter extremes, saved-state round trips,
legacy/invalid state, editor recreation and two-way parameter attachments, plus
the existing saw/MIDI regressions. Build/CTest/pluginval must pass on the actual
artifact. In FL Studio verify listening, automation, editor reopening, two
instances, export tails, new-project save/reopen, and an older fixed-saw project.

## Quality requirements

- Keep audio processing suitable for real-time use; follow AGENTS.md audio rules.
- Avoid stuck notes and invalid numerical output under supported conditions.
- Smooth controls where abrupt changes would cause unwanted clicks.
- Preserve released parameter identifiers and saved-state compatibility.
- Validate performance with a specified build, workload, and machine.
- Require recorded FL Studio listening and host checks for a playable milestone.

## Open product choices

| Choice | Current answer | Needed before |
| --- | --- | --- |
| Synth type and reference sounds | Fixed saw selected for M3; broader direction open | Later sound development |
| First feature set and voice count | Eight-voice MIDI/piano-roll saw with ADSR; no effects/filters | Later sound/polyphony choices |
| MIDI expression and note transitions | Velocity, ADSR, FIFO note-off, bounded stealing; MIDI sustain pedal/bend deferred | Later expression features |
| Distribution and release licensing | Private use now; release route deferred | Distribution |
| Interface style | Native dark editor with four envelope knobs; further refinement open | Interface refinement |
| Performance target and test machine | Unspecified | Benchmark acceptance |

Record settled choices here and add significant rationale to DECISIONS.md.
