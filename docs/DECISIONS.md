# Project decisions

Record significant choices and reasons here. Keep routine progress and current
validation in [STATE](STATE.md). Update [SPEC](SPEC.md) when a requirement changes.

## D001 - Target and stack

- Date: 2026-10-11.
- Status: Established by the user-created AGENTS.md.
- Choice: Windows x64 VST3 for FL Studio, using C++, JUCE, and CMake.
- Reason: Preserve the existing project direction and build a plugin for the
  user's intended host.
- Consequence: Compiler/SDK, framework version, licensing route, and host version
  must be established during setup. Synth sound architecture remains open.

## D002 - Session continuity through a compact handoff

- Date: 2026-10-11.
- Status: Adopted during the requested documentation setup.
- Choice: AGENTS.md directs startup to STATE.md and CODE_MAP.md. STATE is the
  sole current-status tracker; other documents hold focused durable information.
- Reason: The user wants new sessions to continue efficiently without routine
  whole-project exploration.
- Consequence: Agents maintain the checkpoint at meaningful task boundaries,
  verify relevant disk contents, and expand inspection when evidence requires it.

## D003 - Validate incrementally

- Date: 2026-10-11.
- Status: Adopted as the project workflow.
- Choice: Establish the toolchain, then a loadable plugin, then playable behavior;
  implement later features as focused changes with appropriate checks.
- Reason: Each milestone should produce an observable result that makes failures
  easier to locate and progress easier to recover.
- Consequence: Build success, automated checks, and FL Studio listening/host
  checks are recorded separately. Performance claims require measurements.

## D004 - Reuse the installed MSVC toolchain and JUCE modules

- Date: 2026-10-11.
- Status: Adopted for requested environment setup; JUCE runtime validation passed.
- Choice: C++17, Visual Studio 2022 v143, x64, Windows SDK 10.0.26100.0,
  Visual Studio's bundled CMake/CTest, and JUCE 9.0.3 at
  `be29c81492b6151c8ea8d14c840e1311963b3a83`. PowerShell scripts discover CMake
  without changing global PATH. Git on main stores source; generated dependency
  checkouts, downloads, and artifacts are ignored.
- Reason: The compiler, SDK, and CMake are already installed. JUCE's DSP,
  audio/voice/MIDI, parameter, UI, and VST3 facilities cover the initial needs.
  A single framework keeps the dependency setup focused and reproducible.
- Consequence: CMake uses a local verified checkout and performs no downloads.
  The separate setup script retrieves the locked source; a missing dependency
  fails explicitly. CTest establishes compiler/module readiness before M2/M3.
  This does not choose the product's oscillator algorithm or voice architecture.
- References: [JUCE release](https://github.com/juce-framework/JUCE/releases/tag/9.0.3),
  [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/9.0.3/docs/CMake%20API.md).

## D005 - Exact host and private development scope

- Date: 2026-10-11.
- Status: Established by this session's user instructions and private-use reply.
- Choice: FL Studio 21.2.3 build 4004 on Windows x64; private use for now.
  Distribution and its licensing route remain open.
- Reason: The user specified the host version and clarified the initial use.
- Consequence: Host acceptance checks must use this version. The executable
  file version was confirmed; this does not prove plugin compatibility. No
  release licence is assigned and no paid tools are purchased during setup.
  Retain JUCE's upstream notices and revisit its AGPL/commercial route before
  distribution: [upstream licence](https://github.com/juce-framework/JUCE/blob/9.0.3/LICENSE.md).

## D006 - Portable setup within the Windows workspace

- Date: 2026-10-11.
- Status: Adopted while completing the authorized environment installation.
- Choice: Windows curl retrieves pluginval; PowerShell resolves the canonical
  project path through a file handle. Git checks trust only the JUCE directory
  using per-invocation safe.directory, then enforce clean status and exact HEAD.
- Reason: Invoke-WebRequest failed on receive. Switching between sandbox and
  user accounts exposed Git ownership checks; lowercase caller paths failed to
  match Git's Windows path spelling. Global configuration changes are avoidable.
- Consequence: Setup/audit/build scripts operate across those accounts. Existing
  dependencies are preserved, and pluginval's embedded version must match the
  lock. The archive's recorded SHA256 identifies the downloaded file; no upstream
  checksum comparison is claimed. All project build outputs remain in the workspace.

## D007 - Silent instrument foundation and stable host identity

- Date: 2026-10-12.
- Status: Implementation choice for the user's requested discoverable/openable
  sample VST3, following PLAN M2. Original naming superseded by D009; sound open.
- Choice: A JUCE VST3 instrument named Synthesizer v0.1.0, vendor Lazar Audio,
  manufacturer code `Laza`, plugin code `Syba`, bundle ID
  `com.lazaraudio.synthesizer`. No audio input; stereo output; MIDI accepted and
  consumed; a fixed 560x320 native editor. No parameters or persisted settings.
- Reason: Establish host loading and lifecycle before choosing a sound engine.
  Explicit identity codes avoid JUCE's random default plugin code on configure.
- Consequence: Keep identity codes and generated component/controller IDs stable
  when adding the engine so existing FL Studio projects find the same instrument.
  The component CID is `ABCDEF019182FAEB4C617A6153796261`; controller CID is
  `ABCDEF011234ABCD4C617A6153796261`. Parameter IDs and a versioned state format
  must be designed when controls are added; D011 narrows M3 to a fixed sound.
  No VST2 replacement or standalone target is needed.

## D008 - Explicit standard-folder VST3 installation

- Date: 2026-10-12.
- Status: Adopted for the requested FL Studio discovery workflow.
- Physical storage/UAC update policy superseded by D010; standard scan path retained.
- Choice: Build locally with `COPY_PLUGIN_AFTER_BUILD FALSE`; stage the entire
  bundle in artifacts/VST3. Explicit installation targets Windows'
  `Common Files/VST3`, backs up matching existing installations, rejects unrelated
  class IDs, and verifies copied file hashes. pluginval can validate that copy.
- Reason: FL Studio's VST3 scanner uses standard installation locations; custom
  VST search folders do not establish VST3 discovery. Local builds should succeed
  independently of Program Files permissions or a host holding the plugin open.
- Consequence: Installation requires write access to Program Files; GUI validation
  requires an interactive Windows session. The install script requests UAC
  elevation when needed; sandbox approval alone does not grant administrator
  rights. FL Studio scan/load is recorded
  separately from pluginval success. PowerShell waits for pluginval via a pipeline.
- Reference: [Image-Line installation guide](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm).

## D009 - LAZ-SYNTH name and LAZ vendor with preserved identity

- Date: 2026-10-12.
- Status: Name/vendor explicitly selected by the user after confirming the
  original VST3 loads fine in FL Studio.
- Choice: Plugin name, product/bundle/binary name, and editor heading become
  `LAZ-SYNTH`; factory vendor and editor vendor become `LAZ`. The editor reads
  generated JUCE name/vendor macros. Keep the internal CMake target,
  manufacturer/plugin codes, bundle ID, and component/controller CIDs from D007.
- Reason: Apply the requested branding throughout the user-visible plugin while
  keeping its host identity stable for existing projects.
- Consequence: Stage/install LAZ-SYNTH.vst3, verify the renamed artifact and
  CIDs, and move matching old Synthesizer.vst3 bundles to checked workspace
  backup paths only after the replacement succeeds. FL needs a verified rescan
  to refresh metadata. Host project save/reopen after the rename remains a
  separate compatibility check. Version stays 0.1.0; this change adds no sound.

## D010 - User-selected E: storage with a per-plugin FL Studio junction

- Date: 2026-10-12.
- Status: Physical directory explicitly selected by the user during the branding
  update; junction is the implementation choice needed for the selected host.
- Choice: Store bundles in `E:\everything_music_related\VstPlugins`, configured
  by plugin-install.json. One-time setup relocates the original bundle there
  before installing the renamed build, then creates
  `Common Files/VST3/LAZ-SYNTH.vst3` as a junction to the physical E: bundle.
- Reason: The user wants to avoid recurring Program Files access issues. FL
  Studio supports custom VST1/2 paths, while VST3 scans require standard folders.
  A per-plugin junction preserves the standard path without moving other plugins.
- Consequence: One-time migration/link setup needs UAC; subsequent Stage-Plugin
  updates write to E: and do not elevate. Preserve old C: installs in workspace
  backups and old E: installs in same-volume .laz-synth-backups. Native plugin
  loading through the junction is verified separately. The user confirmed the
  renamed installation opens in FL on 2026-10-12; old-project save/reopen remains
  unreported. E: must remain available for the installed junction to work.
- References: [Image-Line folder rules](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/envsettings_files.htm),
  [Microsoft junction semantics](https://learn.microsoft.com/en-us/sysinternals/downloads/junction).

## D011 - First playable milestone is a fixed MIDI saw

- Date: 2026-10-12.
- Status: Scope explicitly selected by the user during planning; implementation
  and recommended voice/velocity/fade defaults remain pending.
- Choice: M3 adds a simple saw playable through MIDI and FL Studio's piano roll,
  with no effects, filters, or adjustable amplitude settings.
- Reason: Establish the smallest useful playable instrument before expanding
  sound design or interface controls.
- Consequence: Replace the silent callback with a saw/voice engine and add musical
  behavior checks. Defer parameters, automation, and a settings format until there
  are controls to persist. Retain project save/reopen and installed-artifact host
  checks. SPEC distinguishes the selected scope from recommended internal defaults.

## Adding or revising a decision

Use the next D-number, date, status, choice, reason, and consequence. Mark a
replaced decision as superseded and point to its replacement. Keep unselected
ideas in SPEC or PLAN rather than presenting them as accepted decisions.
