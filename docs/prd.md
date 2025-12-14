---
stepsCompleted: [1, 2]
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
lastStep: 2
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
