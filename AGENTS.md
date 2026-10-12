# Project
Build a Windows x64 VST3 synthesizer using C++, JUCE, and CMake.
Target host: FL Studio 21.2.3 build 4004. Sound design is still open.

# Session startup
1. Read [STATE](docs/STATE.md) for the current task, last checkpoint, and next action.
2. Read [CODE_MAP](docs/CODE_MAP.md) to locate the relevant files.
3. Follow the next action and read only the supporting documents it names.
4. Verify the relevant files on disk. If Git exists, inspect working-tree changes
   before editing and preserve work from earlier sessions or the user.
5. Expand inspection when the task requires it or the handoff conflicts with disk.
   A routine resumed session should not need a full repository scan.

Treat STATE as a checkpoint, not proof that nothing changed afterward. If a
session was interrupted, inspect its listed work in progress before continuing.

# Where information belongs
- [README](README.md): setup and commands that actually work.
- [SPEC](docs/SPEC.md): requirements, acceptance criteria, and open product choices.
- [PLAN](docs/PLAN.md): milestone sequence and deliverables.
- [STATE](docs/STATE.md): current progress, work in progress, checks, and next action.
- [CODE_MAP](docs/CODE_MAP.md): file locations and responsibilities.
- [DECISIONS](docs/DECISIONS.md): significant choices and their reasons.
- [TESTING](docs/TESTING.md): validation procedure and required evidence.

# Working rules
- Implement focused changes with clear acceptance criteria.
- Preserve unrelated user changes.
- Review and update .gitignore regularly, especially before commits or pushes
  and when tools, dependencies, or generated outputs change.
- Keep only files necessary to build, test, configure, or document the project
  tracked by Git. Exclude generated artifacts, caches, machine-specific settings,
  and secrets. Review already-tracked files too; .gitignore does not untrack them.
- Use the documented build and validation commands. If they do not exist yet,
  establish and verify them before describing them as available.
- Report what passed, failed, or was not run, including the reason.
- Record exact commands and useful failure details; never include credentials.
- Distinguish an agreed requirement from a recommendation or unresolved choice.
- Use one implementation agent by default. Delegate independent tasks when the
  user requests it or applicable instructions explicitly request delegation.

# Progress and handoff
- Update STATE at meaningful checkpoints during long tasks and before ending a
  turn that changes the project. Record unfinished work before handing it off.
- Keep STATE to about 80 lines: replace stale information instead of appending
  a transcript. Keep durable reasoning in DECISIONS and milestone details in PLAN.
- Give the next action a concrete starting point, relevant paths, and a clear
  completion condition. Record pending questions and blockers separately.
- Update CODE_MAP when paths, entry points, commands, or responsibilities change.
- Update SPEC, PLAN, DECISIONS, README, or TESTING only when their facts change.
- Record validation against the actual artifact or source checkpoint checked.
  Earlier passing checks do not validate subsequent changes.

# Audio and compatibility
- Keep allocation, blocking locks, file access, and logging
  out of the audio callback.
- Smooth parameters where abrupt changes would cause clicks.
- Preserve released parameter IDs and saved-state compatibility.
- Measure performance before claiming an optimization.
