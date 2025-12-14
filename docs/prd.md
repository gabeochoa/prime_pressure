---
stepsCompleted: [1, 2, 3]
inputDocuments:
  - docs/GDD.md
  - docs/architecture.md
  - docs/epics.md
  - docs/game-brief-prime_pressure-2025-12-14.md
  - docs/analysis/brainstorming-session-2025-12-14.md
  - docs/project_context.md
  - docs/reference_architecture.md
  - docs/reference_project_context.md
  - docs/ux-design-specification.md
documentCounts:
  briefs: 1
  research: 0
  brainstorming: 1
  projectDocs: 7
workflowType: 'prd'
lastStep: 3
project_name: 'prime_pressure'
user_name: 'gabe'
date: '2025-12-14'
---

# Product Requirements Document - prime_pressure

**Author:** gabe
**Date:** 2025-12-14

## Executive Summary

Prime Pressure is a satirical, retro-desktop warehouse fulfillment game where typing is the core skill and the story is delivered through oppressive systems and the items you ship. The current codebase already supports the core fulfillment loop (order selection → request/fulfill → box/ship) in a pure ECS architecture (C++23 + Raylib + afterhours). This PRD defines the next increments that turn the existing mechanics into a campaign structure with progression, pressure, and meaning—without breaking existing ECS and state machine boundaries.

Planned feature progression (high level):
- Stabilize/clarify the existing core loop (moment-to-moment gameplay)
- Add the Day Loop (start-of-day ritual → shift → end-of-day review/email)
- Add intermediate progression (Gold Stars as a persistent consequence/reward loop)
- Add advanced gameplay (hazard materials and input modifiers as “handling procedures”)
- Flesh out story primarily through items + emails; cutscenes are optional/late
- Add menu UI and settings as a minimal, shippable layer (expand later)

### What Makes This Special

Mechanics are narrative. Difficulty is not arbitrary: pressure systems (TOT, Smile Checks, pledges) and input modifiers (hazard handling) are the story. Players experience “corporate oppression” through interruptions, forced rituals, and escalating item meaning rather than exposition.

## Project Classification

**Technical Type:** game  
**Domain:** gaming  
**Complexity:** medium  
**Project Context:** Brownfield - extending existing system

This PRD focuses on new features that integrate cleanly with the existing ECS patterns and architecture invariants (Sophie is an ECS singleton entity; no C++ singleton accessors).

## Success Criteria

### User Success (player-facing)

- **Primary metric: Day 1–3 vertical slice completion rate**
  - **Campaign (MVP) definition:** the **Day Loop** implemented for **Days 1–3** (start-of-day ritual → shift → end-of-day review/email) with a hard endpoint at **End of Day 3**.
  - **Completion definition:** a player is counted as “complete” when they reach the **End-of-Day 3** screen and successfully advances past it to the “Day 3 Complete” endpoint.
  - **Start definition:** a player is counted as “started” when they complete the first actionable input of Day 1 (i.e., the run is not just a boot-to-menu).
  - **Interpretation metrics (to separate ‘not fun’ from ‘broken’):**
    - **Quit-day distribution:** Day 1 vs Day 2 vs Day 3 exits
    - **Exit reason tags:** voluntary quit vs fired/fail-state vs crash vs suspected softlock

### Business Success (project-facing)

- **Primary gate: “fun enough to keep building”**
  - **Decision rule:** after a small internal playtest set, you still want to keep building *because the Day Loop feels compelling*, not just because it “works.”
  - **Evidence signals (lightweight):**
    - Players voluntarily start another run/day after reaching the Day 3 endpoint (even if it’s “replay Day 3” / “restart run” in the slice build)
    - Feedback clusters around *challenge/fairness/pacing* (good) more than *confusion/controls/state weirdness* (bad)

### Technical Success (non-negotiables)

- **Performance:** 60fps minimum during normal gameplay (no systemic stutter introduced by Day Loop screens).
- **Input latency:** perceived typing responsiveness stays effectively **< 1 frame** (no buffering/focus regressions across view transitions).
- **Stability:** no hard crashes in normal play.
- **Progression integrity:** no known softlocks across Day 1–3 transitions (start-of-day → shift → end-of-day → next day).

### Measurement & Instrumentation (MVP-appropriate)

- **Run validity:** completion metrics should exclude sessions flagged as crash/softlock (tracked separately).
- **Minimum event logging (per session):**
  - session_start / session_end
  - day_start(day=1..3) / day_end(day=1..3)
  - run_completed (day=3) or run_ended(reason=quit|fail|crash|softlock)
  - optional: fps_min/fps_p95 and a “dropped_input_events” counter

## Product Scope

### MVP (Option A) — Day 1–3 Vertical Slice

Deliver a cohesive, replayable **Days 1–3** slice that proves the Day Loop adds meaning and pacing to the existing fulfillment gameplay.

- **Stabilize/clarify the existing core loop**
  - Order selection → fulfill/request → box/ship is understandable, readable, and difficult to break.
- **Day Loop (Days 1–3)**
  - **Start-of-day:** morning pledge ritual (Day 1–3 present; escalation can be light).
  - **Shift:** core work phase contained within the day.
  - **End-of-day:** review/email delivery + day summary; clean transition to the next day until the Day 3 endpoint.
- **Pressure hooks (minimal, to support the premise)**
  - Include at least one “oppression system” interruption within the Day 1–3 slice (e.g., TOT and/or Smile Verification), keeping complexity minimal while validating pacing and state transitions.

**Explicitly out of MVP (Option A):**
- Gold Stars economy and persistent progression spend/earn loop
- Hazard materials / modifier handling rules as “advanced gameplay”
- Cutscenes (optional/late)
- Full menu UI/settings beyond what’s needed to run and playtest comfortably

### Next (Post-MVP)

- **Intermediate progression:** Gold Stars (persistent consequence/reward loop)
- **Advanced gameplay:** hazard materials + input modifiers as handling procedures
- **Story expansion:** deeper email system + item tier progression
- **Menu UI and settings:** expanded, shippable UX for options and accessibility
