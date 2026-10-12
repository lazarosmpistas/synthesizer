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

User parameters, automation, and a versioned settings format are deferred until
controls are introduced. M3 still checks project save/reopen with the fixed sound;
active notes are transient and must not be restored as held voices. The broader
subtractive-synth direction remains open beyond this first oscillator.

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
| First feature set and voice count | MIDI/piano-roll saw, no effects/filters/amp controls; eight voices adopted | Later polyphony choices |
| MIDI expression and note transitions | Velocity, fixed fades, FIFO note-off, bounded stealing adopted; sustain/bend deferred | Later expression features |
| Distribution and release licensing | Private use now; release route deferred | Distribution |
| Interface style | Foundation UI; refinement open | Interface refinement |
| Performance target and test machine | Unspecified | Benchmark acceptance |

Record settled choices here and add significant rationale to DECISIONS.md.
