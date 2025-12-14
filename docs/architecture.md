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

## Project Context Analysis

### Requirements Overview

**Functional Requirements:**
*   **Core Simulation:** High-fidelity typing and "boxing" mechanics (Place, Fold, Tape, Label).
*   **Oppression Systems:** Timer-based pressure (TOT), Interruptions (Smile Verification), and Daily Rituals (Pledges).
*   **Progression:** 20-Day linear campaign with branching endings.
*   **Economy:** Dual currency system (Stars for health, Black Market Points for upgrades).

**Architectural Constraints (User Specified):**
*   **Pure ECS:** All game state, including "Meta" and "UI" where possible, should be represented via Entity-Component-System patterns (`afterhours` library).
*   **Phasing:** Focus on **MVP Gameplay first**. Reactive polish (shaders, dynamic audio) is deferred.
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
*   **Pattern:** Pure ECS (Components in `src/components`, Logic in `src/systems`).
*   **Platform:** macOS Native (Codesigned).

## Core Architectural Decisions

### Decision Priority Analysis

**Critical Decisions (Block Implementation):**
*   **Input Architecture:** Keystroke Sequence Mapping (Keyboard-only Focus).
*   **Meta-Persistence:** "Sophie" Pattern (Singleton Entity).
*   **Render Strategy:** Functional/Direct Mode (Defer Art).

### Input & Interaction Architecture

**Decision:** **Command Sequence Typing**
*   **Pattern:** Gameplay actions are defined by *sequences of keystrokes* (e.g., "PPPBFTS") rather than single hotkeys.
*   **Device:** **Keyboard Only** for core loop. Mouse is restricted to Menu/UI interactions.
*   **Implementation:** `InputSystem` buffers character streams and matches against `OrderConstraint` patterns.

### Meta-Game Persistence ("Sophie" Pattern)

**Decision:** **ECS Singleton Entity**
*   **Pattern:** A single persistent entity (named "Sophie" or similar) acts as the data store for global state.
*   **Components:** `GoldStarManager`, `FactionAffinity`, `InventoryState`.
*   **Rationale:** Maintains "Pure ECS" constraint. Systems query "Sophie" to check global currencies or reputation, rather than accessing a global C++ singleton class.

### Rendering & Polish

**Decision:** **Function over Form (MVP)**
*   **Strategy:** Implement rendering via direct `RenderSystem` calls using primitive shapes/text initially.
*   **Polish:** Post-processing, shaders, and complex art assets are explicitly **deferred** until gameplay loops are solid.
*   **Rationle:** Focus engineering effort on the "Typing Feel" and "Sophie" logic first.

### Decision Impact Analysis

**Implementation Sequence:**
1.  **Sophie Entity:** Create the persistent entity and `GameState` components.
2.  **Input Buffer:** Implement the system to capture and parse "PPPBFTS" sequences.
3.  **Boxing Logic:** Connect Input sequences to State transitions (Box -> Folded).
4.  **UI Overlay:** Add simple text debug UI to show "Sophie's" state (Stars/Affinity).

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
    *   `gen_first()`: For Singletons (Player, Sophie).
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

```
### Complete Project Directory Structure

**Current vs. Target Migration Plan**

We are transitioning from the current "Header-Heavy" structure to a "Cpp-Based" Pure ECS structure.

**1. Current Inventory (To Be Refactored)**
*   `src/systems/*.h` (34+ Header-only Systems) -> **Consolidate** into `src/systems/*.cpp`.
    *   *Example:* Merge `ProcessBoxingInputSystem.h`, `ProcessTypingInputSystem.h` -> `input_system.cpp`.
    *   *Example:* Merge `RenderBoxingViewSystem.h`, `RenderComputerView.h` -> `render_system.cpp`.
*   `src/components.h` & `src/order_components.h` -> **Split** into `src/components/*.h`.
*   `src/order_state_machine.cpp` -> **Port logic** to `boxing_system.cpp`.

**2. Target Structure (Goal)**

```
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
│   ├── systems/            # [REFACTOR] Consolidate 34 files into core logic
│   │   ├── input_system.cpp      # Handles "PPPBFTS" & Buffer
│   │   ├── boxing_system.cpp     # Handles Box/Fold/Tape Logic
│   │   ├── oppression_system.cpp # Handles TOT/Smile Logic
│   │   └── meta_system.cpp       # Handles Day/Save/Sophie Logic
│   └── engine/
│       └── entity_query.h  # Shared Utility (Keep)
```

### Architectural Boundaries

**Input Boundary:**
*   `InputSystem` reads raw keystrokes and updates `InputBuffer` components.
*   It does *not* directly trigger gameplay actions. It produces data that other systems consume.

**Meta Boundary:**
*   `MetaSystem` manages the `Sophie` singleton.
*   Other systems (e.g., Boxing) query `Sophie` to check "Can I box right now?" (Is Day Active?).

**Render Boundary:**
*   `RenderSystem` is Read-Only. It visualizes the current component state.
*   It does *not* modify game state (no "OnClick" callbacks inside the render loop).

### Requirements to Structure Mapping

**Epic 1: Fulfillment** -> `src/systems/boxing_system.cpp` + `src/components/boxing.h`
**Epic 2: Oppression** -> `src/systems/oppression_system.cpp` + `src/components/oppression.h`
**Epic 3: Meta** -> `src/systems/meta_system.cpp` + `src/components/meta.h`

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
    *   **Syntax:** JSON strings use the literal character produced (e.g., `"recipe": "PPP{T}"`). No special modifier codes needed.
    *   **Reserved Keys:** Numbers (`0-9`) are reserved for Order Selection.

**Gap Analysis Results:**
*   **Minor (Deferred):** Glitch Shader implementation details (Deferred to P2).
*   **Resolved:** Input Data Schema defined.

### Architecture Completeness Checklist

**✅ Requirements Analysis**
- [x] Epics mapped to Systems.
- [x] Input requirement (Zero Latency) mapped to `InputBuffer` pattern.

**✅ Architecture Decisions**
- [x] "Sophie" Singleton pattern chosen for Persistence.
- [x] "EntityQuery" pattern chosen for State Access.
- [x] Input Syntax standard defined.

**✅ Project Structure**
- [x] Directories mapped.
- [x] Metadata (Components/Systems) assigned to files.

### Architecture Readiness Assessment

**Overall Status:** READY FOR IMPLEMENTATION

**First Implementation Priority:**
Refactor `src/systems/input_system.cpp` to use the new `InputBuffer` + `^P` (Shift-P) prefix logic.

## Architecture Completion Summary

### Final Architecture Deliverables

**📋 Complete Architecture Document**
*   **Validated**: 2025-12-14
*   **Status**: `complete`
*   **Key Decisions**: Pure ECS, Keyboard-Sequence Input, "Sophie" Persistence, Make Build System.

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
- [x] Meta (Persistence) -> `Sophie` Singleton Entity.
- [x] Oppression (Stress) -> `TOTTimer` Components.

---

**Architecture Status:** READY FOR IMPLEMENTATION ✅

**Next Phase:** Begin implementation using the architectural decisions and patterns documented herein.
