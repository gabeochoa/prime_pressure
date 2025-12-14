---
stepsCompleted: [1, 2, 3, 4, 5, 6, 7, 8]
inputDocuments: 
  - docs/GDD.md
  - docs/epics.md
  - docs/game-brief-prime_pressure-2025-12-14.md
  - docs/analysis/brainstorming-session-2025-12-14.md
  - docs/reference_architecture.md
hasProjectContext: true
workflowType: 'architecture'
lastStep: 8
status: 'complete'
completedAt: '2025-12-14'
project_name: 'prime_pressure'
user_name: 'gabe'
date: '2025-12-14'
---

# Architecture Decision Document

_This document builds collaboratively through step-by-step discovery. Sections are appended as we work through each architectural decision together._

## Architecture Invariants

- **Sophie is ECS**: Sophie is an **ECS singleton entity** used for global/meta-game persistence (currencies, day/run state, etc.).
  - **Prohibited**: Do not introduce a `Sophie::get()` C++ singleton pattern for Prime Pressure.

## Executive Summary

Prime Pressure is a C++23 + Raylib game implemented as a **pure ECS** on the vendored `afterhours` library. The architecture prioritizes low-latency keyboard input, small focused systems, and an ECS-backed global state (“Sophie” as a singleton **entity**) to keep gameplay, meta-progression, and UI consistent and AI-agent-friendly.

This document is aligned to the current PRD MVP target: a **Day 1–3 vertical slice** that proves the Day Loop (pledge → work phase → review/email) integrates cleanly with the existing fulfillment gameplay without sacrificing typing feel or architectural invariants.

## Decision Summary

| Category | Decision | Notes |
|---|---|---|
| Language | C++23 | Enforced by `makefile` (`-std=c++23`). |
| Build | GNU Make | Single-binary build; Raylib pulled via `pkg-config` on non-Windows. |
| Rendering | Raylib | Multi-view rendering via focused render systems; avoid monolithic render loop. |
| ECS | `afterhours` (vendored) | Components are POD-ish `BaseComponent` structs; logic lives in systems. |
| Global state | Sophie = ECS singleton entity | Systems query Sophie entity/components; no C++ singleton accessors. |
| Input | Buffered typing + JSON mapping | Use the single-keystroke JSON syntax; `^` encodes keyboard Shift in data/config, not in gameplay code. |

## Development Environment

- Build: `make`
- Run: `make run`
- Clean: `make clean`
- Dependency: Raylib must be discoverable via `pkg-config` on Linux/macOS (see `makefile`).

## Project Context Analysis

### Requirements Overview

**Functional Requirements:**
*   **Core Simulation:** High-fidelity typing and "boxing" mechanics (Place, Fold, Tape, Label).
*   **Oppression Systems:** Timer-based pressure (TOT), Interruptions (Smile Verification), and Daily Rituals (Pledges).
*   **Progression:** MVP is a **Day 1–3 vertical slice** (hard endpoint at End of Day 3). Full campaign (e.g., 20 days + endings) is a later expansion.
*   **Economy:** Dual currency system (Stars for health, Black Market Points for upgrades).

**Architectural Constraints (User Specified):**
*   **Pure ECS:** All game state, including "Meta" and "UI" where possible, should be represented via Entity-Component-System patterns (`afterhours` library).
*   **Phasing:** Focus on **MVP Gameplay first** (Day 1–3 vertical slice). Reactive polish (shaders, dynamic audio) is deferred.
*   **UI Strategy:** Defer complex UI architecture. Focus on functional gameplay first.

**Metric & Scale:**
*   **Primary domain:** Native C++ Game Dev (Raylib).
*   **Complexity level:** Medium (Complex State Management, Simple Physics).
*   **Estimated architectural components:** ~15-20 Core Systems.

### Technical Constraints & Dependencies

*   **Reference Architecture:** Must match the patterns in `docs/reference_architecture.md`.
*   **Library:** Raylib (Graphics/Windowing), `afterhours` (ECS/Utils).
*   **Input:** Raw Keyboard Input handling required for zero-latency gameplay.

### Cross-Cutting Concerns Identified

1.  **Input Pipeline:** Handling standard text entry vs. "Gameplay Typing" (Hotkeys/Modifiers) vs. UI Navigation.
2.  **Global Simulation State:** Persisting state (Stars, Inventory) across Day boundaries within the ECS.
3.  **UI/World Separation:** Minimal separation initially; focus on world.

## Starter Template Evaluation

### Primary Technology Domain
**Native C++ Game Development** (Raylib + ECS).

### Starter Options Considered
*   **CMake + Raylib Template:** Industry standard for cross-platform, but adds complexity.
*   **Make + Raylib (Current):** Aligns with "Spartan" Raylib philosophy. Simple, direct control.
*   **Premake:** Good middle ground, but unnecessary overhead for single-dev Mac project.

### Selected Foundation: Current 'Prime Pressure' Codebase
**Rationale:** The existing `makefile` is robust, production-ready, and aligns with the user's strong preference for simplicity over complex build generators like CMake. It cleanly supports the required C++23 standard and ECS library structure.

**Architectural Decisions Verified:**
*   **Build:** GNU Make with clang++ (C++23) - Chosen for simplicity.
*   **ECS:** `afterhours` custom library (Vendored).
*   **Pattern:** Pure ECS (Today: components aggregated in `src/components.h` + `src/order_components.h`; systems in `src/systems/*.h`. Target: split components into `src/components/*` and consolidate systems into `src/systems/*.cpp`).
*   **Platform:** macOS Native (Codesigned).

## Core Architectural Decisions

### Decision Priority Analysis

**Critical Decisions (Block Implementation):**
*   **Input Architecture:** Keystroke Sequence Mapping (Keyboard-only Focus).
*   **Meta-Persistence:** "Sophie" Pattern (singleton entity).
*   **Render Strategy:** Functional/Direct Mode (Defer Art).

### Input & Interaction Architecture

**Decision:** **Command Sequence Typing**
*   **Pattern:** Gameplay actions are defined by *sequences of keystrokes* (e.g., "PPPBFTS") rather than single hotkeys.
*   **Device:** **Keyboard Only** for core loop. Mouse is restricted to Menu/UI interactions.
*   **Implementation:** `InputSystem` buffers character streams and matches against `OrderConstraint` patterns.

**Input implementation notes (Prime Pressure)**
*   Treat each frame as authoritative: poll input every frame, append to `InputBuffer`, and let downstream systems consume buffer state.
*   Do not rely on OS repeat-rate for gameplay feel; gameplay timing should be driven by dt + your own buffering rules.
*   Keyboard-Shift-modified commands are represented via the JSON `^` syntax in config/data; do not check keyboard Shift directly in gameplay code.

### Meta-Game Persistence ("Sophie" Pattern)

**Decision:** **ECS Singleton Entity**
*   **Pattern:** A single persistent entity (named "Sophie" or similar) acts as the data store for global state.
*   **Components (MVP):** `CampaignProgress` (day/phase + slice endpoint), `RunTelemetryState` (session/run identifiers + counters), plus any minimal day-loop state needed for gating input.
*   **Components (Later):** `GoldStars`, `MarketPoints`, `FactionAffinity`, richer inventory/meta state.
*   **Rationale:** Maintains "Pure ECS" constraint. Systems query "Sophie" to check global currencies or reputation, rather than accessing a global C++ singleton class.

### Day Loop & Campaign Slice (MVP: Days 1–3)

**Decision:** Model the Day Loop as explicit ECS state (stored on Sophie) and drive it via small, focused systems.

**Core concept:**
- A “day” is a structured sequence of phases:
  - **Start-of-day:** pledge ritual
  - **Work phase:** normal fulfillment gameplay (existing loop)
  - **End-of-day:** review/email + day summary
- The MVP slice hard-ends at **End of Day 3**.

**Proposed ECS state (stored on the Sophie singleton entity):**
- `CampaignProgress`
  - `int day_index` (1..3 for MVP)
  - `enum DayPhase { Pledge, Work, Review }`
  - `bool slice_complete` (true at End of Day 3)
- `RunTelemetryState`
  - `uint64_t session_id` (or string)
  - `uint64_t run_id`
  - counters/flags: `crash_flag`, `softlock_flag`, `last_exit_reason`, and a small “event counters” struct (see Instrumentation section below)

**Phase gating rule (architectural):**
- Gameplay systems that mutate fulfillment state should only run when `CampaignProgress.phase == Work`.
- Pledge/review systems run only in their phases.
- Render systems remain read-only (they can display phase UI based on Sophie state).

### Phase Responsibility Matrix (Pledge / Work / Review)

This is the “no ambiguity” contract for which systems can run, what input is accepted, and what gets cleared at phase boundaries.

| Day phase | Player intent | Systems allowed to mutate gameplay state | Allowed inputs | Required buffer / state behavior |
|---|---|---|---|---|
| **Pledge** | Type the pledge to begin the day | Pledge system + meta/day-loop system only | Pledge typing only | Work-phase typing/boxing buffers must not accidentally dismiss pledge; completion moves to Work |
| **Work** | Fulfillment gameplay (orders → typing → boxing/shipping) | Fulfillment systems + oppression systems (TOT, optional Smile) + meta/day-loop | Normal gameplay typing + view navigation | TOT runs only here; transitions out must not “eat” buffered characters |
| **Review** | See end-of-day summary/email and continue | Review/email system + meta/day-loop system only | Confirm/continue inputs only | On entry, flush/ignore leftover Work-phase buffers so stray keystrokes don’t auto-advance |

### Rendering & Polish

**Decision:** **Function over Form (MVP)**
*   **Strategy:** Implement rendering via direct `RenderSystem` calls using primitive shapes/text initially.
*   **Polish:** Post-processing, shaders, and complex art assets are explicitly **deferred** until gameplay loops are solid.
*   **Rationale:** Focus engineering effort on the "Typing Feel" and "Sophie" logic first.

### Decision Impact Analysis

**Implementation Sequence:**
1.  **Sophie Entity:** Create the persistent entity and MVP meta components (`CampaignProgress`, `RunTelemetryState`).
2.  **Day Loop:** Implement the Day 1–3 phase state machine (Pledge → Work → Review) with a hard endpoint at End of Day 3.
3.  **Input Buffer:** Preserve typing feel while ensuring view/phase transitions never drop or “eat” characters.
4.  **Minimal pressure hook:** Add **TOT** (recommended for MVP) gated to Work phase only. (Smile can be added later without changing the Day Loop structure.)
5.  **Debug UI:** Add simple text UI to show Day/Phase and basic telemetry counters for playtests.

**Cross-Component Dependencies:**
*   `InputSystem` needs to know if `Sophie` allows inputs (e.g., if Day is active).
*   `RenderSystem` needs to visualize the `InputBuffer` so players see what they typed.

## Implementation Patterns & Consistency Rules

### Pattern Categories Defined

**Critical Conflict Points Identified:**
3 areas where AI agents could make different choices (Logic placement, Query syntax, Naming).

### Component & Logic Patterns

**Decision:** **Lightweight Rich Components**
*   **Pattern:** Components are primary POD (Plain Old Data) structs.
*   **Functions:** Helper functions (`is_folded()`, `mark_active()`) are allowed for syntax sugar/getters.
*   **Constraint:** *Heavy logic* (complex calculations, iterations) MUST reside in Systems.

### Query Patterns

**Decision:** **Fluent EntityQuery Interface**
*   **Pattern:** Use the custom `EntityQuery()` builder for looking up entities.
*   **Syntax:** `EntityQuery().whereHasComponent<T>().gen()`
*   **Guidance:**
    *   `gen_first()`: For singleton entities (Player, Sophie).
    *   `gen_ids()`: If only ID is needed (faster).
    *   **Prohibited:** Do NOT iterate `EntityQuery` inside another tight loop (O(N^2)).

### Naming Patterns

**Decision:** **Snake Case (Standard C++)**
*   **Variables:** `variable_name` (Not `m_variableName`).
*   **Functions:** `do_something()` (Not `DoSomething` or `doSomething`).
*   **Components:** `PascalCase` struct names (`Transform`), `snake_case` file names (`transform.h`).

### UI Architecture Update

**Correction:** The `afterhours` library contains an ECS-based UI system.
*   **Pattern:** UI elements are Entities with UI components (`Button`, `Label`).
*   **Logic:** `UISystem` processes interactions and rendering natively within the main loop.

## Project Structure & Boundaries

### Complete Project Directory Structure

**Current vs. Target Migration Plan**

We are transitioning from the current "header-heavy" structure to a "cpp-based" pure ECS structure.

**1. Current inventory (today)**

- Systems: `src/systems/*.h` (header-only systems; e.g., `ProcessTypingInputSystem.h`, `RenderBoxingViewSystem.h`)
- Components: `src/components.h` and `src/order_components.h` (aggregated headers)
- State machine: `src/order_state_machine.cpp`
- Views/render: `src/render_*.cpp` + render systems under `src/systems/Render*`

**2. Target structure (goal)**

```text
prime_pressure/
├── makefile                # Existing (Keep)
├── src/
│   ├── main.cpp            # Entry Point (Keep)
│   ├── game.cpp            # App Loop (Keep)
│   ├── components/         # [NEW] Split logical components here
│   │   ├── boxing.h        # (Refactor from order_components.h)
│   │   ├── oppression.h    # (Refactor from components.h)
│   │   ├── meta.h          # (New Sophie Components)
│   │   └── ui.h            # (New UI Components)
│   ├── systems/            # [REFACTOR] Consolidate into core logic
│   │   ├── input_system.cpp      # Typing buffer + command parsing
│   │   ├── boxing_system.cpp     # Box/Fold/Tape/Label workflow
│   │   ├── oppression_system.cpp # TOT/Smile/Pledge interruptions
│   │   └── meta_system.cpp       # Day cycle + Sophie persistence
│   └── engine/             # Shared utilities (if/when separated)
│       └── entity_query.h
```

### Architectural Boundaries

**Input Boundary:**
*   `InputSystem` reads raw keystrokes and updates `InputBuffer` components.
*   It does *not* directly trigger gameplay actions. It produces data that other systems consume.

**Meta Boundary:**
*   `MetaSystem` manages the `Sophie` singleton entity.
*   Other systems (e.g., Boxing) query `Sophie` to check "Can I box right now?" (Is Day Active?).

**Telemetry Boundary (MVP-friendly):**
*   A small `TelemetrySystem` (or equivalent utility) records **coarse session/run events** (day_start/day_end/run_completed/run_ended).
*   Telemetry must not be required for gameplay correctness; it is observational and safe to disable.

**Render Boundary:**
*   `RenderSystem` is Read-Only. It visualizes the current component state.
*   It does *not* modify game state (no "OnClick" callbacks inside the render loop).

### Requirements to Structure Mapping

**Current mapping (today)**

| Epic | Current core code (authoritative) | Current systems/files (update loop) | Current systems/files (render loop) |
|---|---|---|---|
| **E01 – Core Fulfillment Loop (P0)** | `src/order_components.h` (OrderWorkflow + state enums + tags + count components)<br>`src/order_state_machine.*` (state table + transitions)<br>`src/components.h` (Order, TypingBuffer, BoxingProgress, ConveyorItem, view + selection singletons) | `src/systems/SpawnItemsSystem.h` (spawn shelf items)<br>`src/systems/GenerateOrdersSystem.h` (spawn orders + workflow/count components)<br>`src/systems/ManageInProgressOrderTagSystem.h` (derives `IsInProgressOrder` tag from `OrderSlot`)<br>`src/systems/ManageSelectedOrderTagSystem.h` (derives `IsSelectedOrder` tag from SelectedOrder singleton)<br>`src/systems/ProcessOrderSelectionSystem.h` (1–9 select/open orders; resets typing buffer)<br>`src/systems/ProcessOrderTabbingSystem.h` (TAB cycle active order)<br>`src/systems/ProcessTypingInputSystem.h` (character buffer + timeout + status)<br>`src/systems/MatchItemToOrderSystem.h` (warehouse “request item” input → requested counts + conveyor move)<br>`src/systems/SpawnConveyorItemsSystem.h` (spawn conveyor items per order)<br>`src/systems/ManageConveyorItemsSystem.h` (move conveyor items; mark items ready + received counts)<br>`src/systems/ProcessBoxingInputSystem.h` (boxing key sequence; syncs OrderWorkflow to BoxingProgress)<br>`src/systems/ProcessReadyStampSystem.h` (READY/TO/SHIP stamping states)<br>`src/systems/ManageOrderStateTagsSystem.h` (keeps macro/micro state tags in sync)<br>`src/systems/UpdateOrderWorkflowSystem.h` (advances processing states via state machine)<br>`src/systems/DebugOrderWorkflowSystem.h` (periodic workflow logging)<br>`src/systems/GrabItemSystem.h` + `src/systems/BoxItemSystem.h` (WIP: grabbed → boxed; needs boxed-count wiring) | `src/systems/RenderComputerView.h` (order cards + timeline rendering)<br>`src/systems/RenderWarehouseView*.h` (warehouse belt + request prompt rendering)<br>`src/systems/RenderBoxingViewSystem.h` (boxing UI + prompts)<br>`src/systems/RenderTypingBufferSystem.h` (status strip + hints)<br>`src/systems/RenderRenderTextureSystem.h` + `src/systems/UpdateRenderTextureSystem.h` (present / resize render textures)<br>`src/render_*.cpp` + `src/render_views.h` (view registration) |
| **E02 – Oppression Systems (P0)** | `docs/GDD.md` + `docs/epics.md` define TOT / Smile / Pledge requirements | *(Not implemented yet in `src/systems/`)*<br>**MVP subset required:** Morning pledge gate + **TOT** (recommended) integrated with the Day Loop phases (Work-only).<br>Planned: systems/components for TOT timer, Smile verification interruptions, and pledge gate. | *(Not implemented yet in `src/systems/`)* |
| **E03 – Economy & Meta (P1)** | `src/settings.*` (save/load + settings persistence)<br>`src/preload.*` (startup + resource setup) | *(Not implemented yet as “Sophie” ECS state)*<br>**MVP subset required:** Sophie singleton entity + `CampaignProgress` (Day 1–3, phase state) + minimal run/session telemetry state.<br>Current global-ish runtime state is handled via afterhours singleton components created in `src/game.cpp` (e.g., `TypingBuffer`, `ActiveView`, `SelectedOrder`, `ActiveOrder`, `BoxingProgress`). Planned: introduce a dedicated Sophie singleton **entity** for currencies/day-cycle/meta. | *(N/A)* |

**Target mapping (goal)**

- **Epic 1: Fulfillment** -> `src/systems/boxing_system.cpp` + `src/components/boxing.h`
- **Epic 2: Oppression** -> `src/systems/oppression_system.cpp` + `src/components/oppression.h`
- **Epic 3: Meta** -> `src/systems/meta_system.cpp` + `src/components/meta.h`

## Architecture Validation Results

### Coherence Validation ✅

**Decision Compatibility:**
The stack (C++23 + Make + custom ECS) is internally coherent and maintained by the user. "Pure ECS" logic placement is enforced.

**Structure Alignment:**
The flat `src/systems` structure matches the MVP requirements. Separation of `components` and `systems` is correct for ECS.

### Implementation Readiness Validation ✅

**Missing Definitions (Resolved):**
*   **Gap (Resolved): Input Constraints in Data (`items.json`)**
    *   **Decision:** Game input is primarily **Single Keystroke** (Letters & Unshifted Symbols).
    *   **Progression:** **Shifted Symbols** (e.g., `{`, `}`, `?`) are permitted for **Late Game** difficulty spikes.
    *   **Syntax:** Use the **single-keystroke JSON syntax** where `^` encodes keyboard Shift in data/config (not in gameplay code).
        *   Example: `"recipe": "PPP^T"` means the final key is `keyboard Shift+T`.
        *   For shifted symbols, encode the *base key* with `^` (e.g., `^[` for `{`, `^/` for `?`) rather than placing raw `{` / `?` in gameplay recipes.
    *   **Reserved Keys:** Numbers (`0-9`) are reserved for Order Selection.

**Gap Analysis Results:**
*   **Minor (Deferred):** Glitch Shader implementation details (Deferred to P2).
*   **Resolved:** Input Data Schema defined.

### Architecture Completeness Checklist

**✅ Requirements Analysis**
- [x] Epics mapped to Systems.
- [x] Input requirement (Zero Latency) mapped to `InputBuffer` pattern.

**✅ Architecture Decisions**
- [x] Sophie singleton entity chosen for persistence.
- [x] "EntityQuery" pattern chosen for State Access.
- [x] Input Syntax standard defined.

**✅ Project Structure**
- [x] Directories mapped.
- [x] Metadata (Components/Systems) assigned to files.

### Architecture Readiness Assessment

**Overall Status:** READY FOR IMPLEMENTATION

**First Implementation Priority:**
Implement the **Day 1–3 vertical slice foundation** without risking typing feel:
1. Create the **Sophie** singleton entity with `CampaignProgress` + minimal `RunTelemetryState`.
2. Implement the Day Loop phases (Pledge → Work → Review) and hard endpoint at **End of Day 3**.
3. Keep `InputBuffer` behavior stable across phase/view transitions (no dropped characters), continuing to honor the JSON `^` keyboard-Shift-encoding rules (no direct keyboard Shift checks in gameplay code).
4. Add **TOT** as the minimal “pressure hook,” gated to Work phase only.

## Architecture Completion Summary

### Final Architecture Deliverables

**📋 Complete Architecture Document**
*   **Validated**: 2025-12-14
*   **Status**: `complete`
*   **Key Decisions**: Pure ECS, Keyboard-Sequence Input, Sophie singleton entity persistence, Make build system.

**🏗️ Implementation Ready Foundation**
*   **Build**: Use existing `makefile`.
*   **ECS**: Use existing `afterhours` vendor library.
*   **Structure**: Follow the map in "Project Structure" (Flat Systems).

**📚 AI Agent Implementation Guide**
1.  **Read this document** before implementing any feature.
2.  **Respect the Boundaries**: Do not put logic in Components (except simple getters). Do not put rendering logic in Gameplay systems.
3.  **Follow Naming**: `snake_case` for everything except Component Structs.
4.  **Reference Pattern**: Use `EntityQuery` for all lookups.

### Quality Assurance Checklist

**✅ Architecture Coherence**
- [x] All decisions work together without conflicts.
- [x] `afterhours` ECS patterns are respected.

**✅ Requirements Coverage**
- [x] Input (Zero Latency) -> `InputBuffer` Component.
- [x] Meta (Persistence) -> Sophie singleton entity.
- [x] Oppression (Stress) -> `TOTTimer` Components.

---

**Architecture Status:** READY FOR IMPLEMENTATION ✅

**Next Phase:** Begin implementation using the architectural decisions and patterns documented herein.
