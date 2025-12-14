# Project Context
\n---\n
## reference_project_context

# kart chaos

## Critical Rules

### Architecture & ECS
- **Core Library**: Must use the `afterhours` vendor library for ECS functionality.
- **Fluent Queries**: Use `EntityQuery` (or `EQ()`) with fluent syntax for ad-hoc entity retrieval.
    - Example: `EntityQuery().whereHasComponent<T>().whereLambda(...).gen_first()`
- **"Sophie" Singleton**: Use the "Sophie" pattern for global/meta-game state persistence. Access singleton components via `EntityHelper::get_singleton_cmp<T>()`.
- **Functionality-First Rendering**: Rendering logic is split into specific systems (e.g., `RenderSpritesWithShaders`, `RenderEntities`) rather than a monolithic render loop.

### Coding Standards
- **Standard**: C++23 is strictly enforced.
- **Naming Conventions**:
    - **Variables/Functions**: `snake_case` (e.g., `get_spawn_position`, `running`).
    - **Types/Classes/Components**: `PascalCase` (e.g., `SystemManager`, `Transform`, `EntityAndRegistry`).
    - **Constants**: `snake_case` or `kCamelCase` (mixed usage, prefer consistent `snake_case` for new constants).
- **Ownership**: Use `std::unique_ptr` for system registration and ownership transfer. Avoid raw pointers for ownership.

### Input Handling
- **Single Keystroke Syntax**: Input configuration uses a specific JSON syntax where `^` denotes the Shift modifier.
    - Example: `^P` represents `Shift + P`.
    - Do NOT use "Shift+P" or other variations.

## Implementation Patterns

### Component Definition
Components must inherit from `::afterhours::BaseComponent` and should be primarily Plain Old Data (POD) structs.
```cpp
struct Transform : ::afterhours::BaseComponent {
    vec2 position{0.f, 0.f};
    vec2 velocity{0.f, 0.f};
    // Helper methods allowed for data manipulation
    vec2 center() const { ... }
};
```

### System Definition
Systems inherit from `afterhours::System<Components...>` to define required components for iteration.
```cpp
struct MoveSystem : afterhours::System<Transform, Velocity> {
    virtual void for_each_with(Entity &entity, Transform &t, Velocity &v, float dt) override {
        // Logic here
    }
};
```

### Rendering Pipeline
The rendering pipeline is multi-pass, utilizing `RenderTexture2D`:
1.  **World Render**: Entities, map elements.
2.  **Tag Shader**: Special effects (e.g., spotlight).
3.  **Post Processing**: Global screenspace effects (CRT, chromatic aberration).
4.  **UI**: HUD and debug overlays on top.

### Input Mapping
Inputs are mapped to abstract `InputAction` enums rather than checking keys directly in game logic.
```cpp
if (action_matches(input.action, InputAction::Boost)) { ... }
```

## Anti-Patterns (What NOT to do)

- **Native Shift Checks**: Do NOT check for `IsKeyDown(KEY_LEFT_SHIFT)` directly in gameplay code for command inputs. Rely on the input mapping system and the `^` syntax.
- **Monolithic Systems**: Avoid creating "God Systems" that do everything. Break down logic into small, focused systems (e.g., `UpdateSpriteTransform`, `UpdateAnimationTransform`).
- **Manual Memory Management**: Avoid `new`/`delete`. Use smart pointers and containers (`std::vector`, `std::optional`).
- **Ignoring C++23 Features**: Do not use outdated C++ patterns when C++23 alternatives (like `std::ranges`, `std::format` via `fmt`, etc.) are available.



# pharmasea 

# Project Context: 

## Critical Rules

- **ECS Patterns**
  - Use the **afterhours** library for all entity management. All queries must be performed via the fluent `EntityQuery` API (see `src/entity_query.h`).
  - The **Sophie** singleton pattern is the canonical way to access global game state (see `src/system/sophie.cpp`). Access via `Sophie::get()` – never store raw pointers to the singleton.
  - Components must be **PascalCase** structs inheriting from `BaseComponent` (see `src/components/*.h`).
  - Systems operate on `RefEntity`/`OptEntity` returned by `EntityQuery` and must not manually iterate over raw containers.

- **Coding Style**
  - Variable and function names use **snake_case** (e.g., `render_floating_name`).
  - Component and type names use **PascalCase** (e.g., `HasSubtype`, `CanBeHeld`).
  - All headers start with `#pragma once` and include guards are not used.
  - Prefer early returns for error handling; avoid deep nesting.
  - Do not use `auto` for non‑template types – write explicit types.

- **Input Handling**
  - Input is defined using the **Single Keystroke JSON** syntax. Example: `{"key":"^P"}` represents **Shift‑P**. The caret (`^`) denotes the Shift modifier.
  - **Never** encode Shift modifiers directly in code; always use the JSON representation and let the input system translate it.

- **Constraints**
  - **No `Shift` modifiers** may appear in source code – they must be expressed only via the JSON syntax.
  - Target **C++23** features: structured bindings, `std::expected`, `co_await` for async tasks, and `constexpr` lambdas are encouraged.
  - Avoid deprecated C++20 features such as `std::bind` and raw `new/delete` – use smart pointers and `std::make_unique`.
  - Do not use `std::format` (not yet available in the toolchain) – rely on `fmt` library instead.

## Implementation Patterns

- **EntityQuery Usage**
  ```cpp
  // Example: find the first Sophie entity
  auto sophie = EntityQuery()
                 .whereType(EntityType::Sophie)
                 .gen_first();
  ```
  (see `src/system/sophie.cpp` line 39)

- **Sophie Singleton**
  ```cpp
  class Sophie {
   public:
    static Sophie& get();
    // ...
  };
  // Access pattern
  Sophie::get().do_something();
  ```
  (see `src/system/sophie.cpp` line 117)

- **Component Declaration**
  ```cpp
  struct HasSubtype : public BaseComponent {
    // tag‑only component – no members
  };
  ```
  (see `src/components/has_subtype.h`)

- **System Signature**
  ```cpp
  void render_simple_normal(const Entity& entity, float dt);
  // All system functions live in `src/system/*.h` and follow the same signature.
  ```
  (see `src/system/rendering_system.h`)

- **Input JSON Example**
  ```json
  { "key": "^P" }
  ```
  This JSON is parsed by the input system and translated to a Shift‑P keystroke.

## Anti‑Patterns (What NOT to do)

- **Directly accessing Shift modifiers in code** – e.g., `if (IsKeyPressed(KEY_LEFT_SHIFT))` is prohibited.
- **Manual iteration over entity containers** – always use `EntityQuery`.
- **Using `auto` for concrete types** – write the full type name.
- **Mixing naming conventions** – do not use `camelCase` for variables or `snake_case` for components.
- **Hard‑coding singleton pointers** – never store raw `Sophie*`.
- **Using deprecated C++20 features** – avoid `std::bind`, raw `new/delete`, and `std::format`.
- **Placing system logic in UI code** – keep UI handling in the input system, not in rendering or game‑logic systems.

---
*Generated from analysis of `src/entity_query.h`, `src/system/sophie.cpp`, component headers, and project conventions in `PROJECT_RULES.md`.*
\n---\n
## architecture
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
\n---\n
## reference_architecture
Pharmasea Architecture Report
1. ECS Structure
The project uses a custom or vendored Entity Component System (ECS) library called afterhours.

Entities: Managed by afterhours::Entity (typedef'd in 
src/entity.h
).
Entities hold a set of components, a bitset for presence, and tags.
Serialization is handled via bitsery with a custom 
serialize
 template.
Components: Defined as structs, typically inheriting from BaseComponent (though not strictly required directly, many do).
Example: 
Transform
 in 
src/components/transform.h
.
They contain data and utility methods (e.g., 
update
, 
sync
).
Polymorphism is supported via the afterhours library (e.g., register_polymorphic_component).
Systems: Managed by 
SystemManager
 (singleton) wrapping afterhours::SystemManager.
Systems function as stateless logic processors.
Example: 
process_ai_drinking
, 
process_ai_waitinqueue
 in 
src/system/ai_system.cpp
.
Functions iterate over entities (often using helper methods like find_if_missing or simply checking components).
Registration:
Components are registered in register_all_components() (called in 
src/game.cpp
).
Systems are registered in groups (e.g., register_afterhours_systems, register_sixtyfps_systems) within 
SystemManager
.
2. Game Loop
The application uses Raylib for the windowing and main loop, wrapped in an 
App
 class.

Entry Point: 
main
 in 
src/game.cpp
 calls App::get().run().
Loop Structure:
while (running && !raylib::WindowShouldClose())
Time Step: Variable delta time (dt) via raylib::GetFrameTime().
Layer Stack: Use of a 
Layer
 pattern (
src/engine/layer.h
).
App::loop(dt) iterates through a stack of layers (e.g., GameLayer, MenuLayer, FPSLayer).
Update: layer->onUpdate(dt)
Draw: layer->onDraw(dt) into a main Render Texture.
Rendering:
Draws to an intermediate RenderTexture (mainRT) to allow for post-processing shaders.
Blits mainRT to the screen in 
render_to_screen()
.
3. Project Structure
The codebase follows a standard C++ project layout:

/
├── makefile             # Build script
├── src/                 # Source code
│   ├── components/      # Component definitions (ECS)
│   ├── system/          # System implementations (ECS)
│   ├── engine/          # Core engine (App, Input, Layers, Util)
│   ├── network/         # Networking logic
│   ├── layers/          # Application layers (Game, Menu, UI)
│   ├── ui/              # UI widgets and theme
│   ├── log/             # Logging utilities
│   ├── game.cpp         # Entry point (main)
│   └── entity.h         # Entity typedefs
├── vendor/              # Third-party libraries
│   ├── afterhours/      # ECS library
│   ├── raylib/          # (Linked/Vendored)
│   └── ...
└── resources/           # Game assets (images, sounds, etc.)
4. Build System
Tool: Standard GNU Make (
makefile
).
Compiler: defaults to clang++.
Dependencies: Uses pkg-config to locate raylib.
Flags:
Standard: -std=c++2a (C++20).
Directives: -DNDEBUG (release), -DTRACY_ENABLE (profiling).
Sanitizers: Mentions -fsanitize=address (commented out).
Features:
Precompiled Headers (
src/pch.hpp
).
Automatic dependency generation (.d files).
Support for ccache.
5. Coding Conventions
Naming:
Files: snake_case.cpp/h (e.g., 
ai_system.cpp
, 
game.cpp
).
Classes/Structs: PascalCase (e.g., 
Transform
, 
SystemManager
, AICleanVomit).
Functions/Variables: snake_case (e.g., 
process_ai_drinking
, register_all_components).
Macros: UPPER_SNAKE_CASE (e.g., ENABLE_DEV_FLAGS).
Modern C++:
Heavy use of C++20 features (implied by -std=c++2a).
auto type deduction.
Lambdas (e.g., in App::onEvent bindings).
Memory Management:
std::shared_ptr and std::unique_ptr used alongside afterhours ECS memory handling.
Entities use StdSmartPtr for components in serialization.
Dependencies:
Bitsery: Used for serialization (networking/saving).
Tracy: Integrated for profiling (TRACY_ZONE_SCOPED).
Log: Custom logging macros (log_info, log_warn).


# CART CHAOS 

## 1. ECS (Entity Component System) Structure

The project relies on a custom ECS implementation provided by the `afterhours` vendor library.

### Components
*   **Definition**: Components are defined as `structs` inheriting from `::afterhours::BaseComponent`.
*   **Storage**: They are Plain Old Data (POD) structures, optionally with helper methods.
*   **Example**:
    ```cpp
    struct Transform : ::afterhours::BaseComponent {
      vec2 position{0.f, 0.f};
      // ...
    };
    ```

### Systems
*   **Definition**: Systems inherit from `afterhours::System<Components...>` or specialized base classes like `PausableSystem`.
*   **Logic**:
    *   `for_each_with`: Iterates over entities possessing specific components.
    *   `once`: Runs once per frame (good for global logic).
*   **Registration**: Systems are manually registered in `main.cpp` via a `SystemManager` instance.
    ```cpp
    systems.register_update_system(std::make_unique<Move>());
    systems.register_render_system(std::make_unique<RenderEntities>());
    ```

### Queries & Entities
*   **Entity Access**: `EntityHelper::get_entities()` provides access to all entities.
*   **Queries**: The `EntityQuery` builder pattern is used for ad-hoc querying.
    ```cpp
    auto opt = afterhours::EntityQuery({...})
               .whereHasComponent<SomeComponent>()
               .gen_first();
    ```

---

## 2. Game Loop

The game loop utilizes **Raylib** and is structured within `main.cpp` using a `SystemManager` to handle execution order.

### Loop Structure
```cpp
while (running && !raylib::WindowShouldClose()) {
  systems.run(raylib::GetFrameTime());
}
```

### Execution Order
1.  **Fixed Update**: Physics/Gameplay logic (registered via `register_fixed_update_system`).
2.  **Update**: General game logic (registered via `register_update_system`).
3.  **Render**: Rendering logic (registered via `register_render_system`).

### Rendering Pipeline
The renderer uses a multi-pass approach with `RenderTexture2D`:
1.  **World Render**: Draws entities, skids, and map elements.
2.  **Tag Shader**: Applies special effects (like the tagger spotlight).
3.  **Post Processing**: Applies global screenspace shaders (CRT, chromatic aberration, etc.).
4.  **UI**: Draws HUD and overlays on top.

---

## 3. Project Structure

The project follows a standard C++ layout with a clear separation of source and dependencies.

```text
├── src/                  # Game Source Code
│   ├── ui/               # UI specific components and systems
│   ├── log/              # Logging utilities
│   ├── components.h      # Component definitions
│   ├── systems.h         # System definitions
│   ├── main.cpp          # Entry point and loop
│   └── ...               # Various managers (Sound, Map, Translation)
├── include/              # (Not heavily used, headers are in src/)
├── resources/            # Game Assets (Images, Sounds, Shaders, Fonts)
├── tools/                # Build and dependency analysis tools
├── vendor/               # Third-party Dependencies
│   ├── afterhours/       # Custom Core Library (ECS, Plugins)
│   ├── raylib/           # Graphics Library
│   ├── fmt/              # String formatting
│   ├── nlohmann/         # JSON parsing
│   ├── backward/         # Stack trace handling
│   └── ...
├── makefile              # Primary build script
├── xmake.lua             # Alternative build script
└── output/               # Build artifacts (binaries, copied resources)
```

---

## 4. Build System

The primary build tool is **Make**, with **xmake** available as a modern alternative.

### Makefile Analysis
*   **Compiler**: `clang++` (macOS) or `g++` (Windows).
*   **Standard**: C++23 (`-std=c++23`).
*   **Flags**: Strict warnings (`-Wall -Wextra`) and AddressSanitizer (`-fsanitize=address`) on macOS.
*   **Workflow**:
    1.  Compiles `src/*.cpp` and `vendor/**/*.cpp`.
    2.  Links against `raylib` (via `pkg-config` or local paths).
    3.  Copies resources to the `output/` directory.
    4.  Signs the executable (on macOS).

### Dependencies
The project statically compiles most vendor libraries (like `fmt`, `afterhours`) directly into the executable or links them as static archives.

---

## 5. Coding Conventions

The codebase follows modern C++ practices.

*   **Standard**: **C++23** is strictly enforced.
*   **Naming**:
    *   **Variables/Functions**: `snake_case` (e.g., `get_spawn_position`, `running`).
    *   **Types/Classes**: `PascalCase` (e.g., `SystemManager`, `Transform`, `EntityAndRegistry`).
    *   **Constants**: `snake_case` or `kCamelCase` (mixed usage).
*   **Memory Management**:
    *   Use of `std::unique_ptr` and `std::make_unique` for ownership transfer (especially systems).
    *   `std::vector` and `std::optional` are preferred over raw arrays/pointers.
*   **Formatting**: Handled via `.clang-format`.
*   **Namespaces**: Heavy use of the `afterhours` namespace for core functionality.
*   **Headers**: Uses `#pragma once` for include guards.
\n---\n
## GDD
# Prime Pressure - Game Design Document

**Author:** Gabe
**Game Type:** Simulation
**Target Platform(s):** PC (Steam, Itch.io)

---

## Executive Summary

### Core Concept

A satirical warehouse fulfillment game where you type to fill orders while navigating an increasingly dystopian corporate environment that shifts from "business as usual" to a "war economy."

### Target Audience

Fans of "job simulator" games with deep narrative layers (*Papers, Please*, *Not Tonight*) and high-skill typing/management games (*Cook, Serve, Delicious!*).

### Unique Selling Points (USPs)

1.  **Mechanics as Narrative:** The typing gets harder *because* the world gets worse (Modifiers), not just arbitrary difficulty.
2.  **The "Glitch" Aesthetic:** The UI and Music degrade over time, mirroring the protagonist's mental state.
3.  **Satire of "Now":** Directly addresses the current cultural conversation around fulfillment centers and labor rights.

---

## Goals and Context

### Project Goals

To create a visceral, "stress-fun" experience that uses typing mechanics to critique the human cost of modern convenience, blending the frantic gameplay of *Cook, Serve, Delicious!* with the narrative oppression of *Papers, Please*.

### Background and Rationale

The "workplace dystopia" genre has a dedicated following. Players enjoy the tension between mastering complex mechanics and navigating a hostile narrative setting. *Prime Pressure* capitalizes on this by gamifying the familiar (and often criticized) experience of warehouse fulfillment.

---

## Core Gameplay

### Game Pillars

1.  **Narrative Action:** Mechanical inputs (typing) are directly tied to narrative tension. Complex inputs represent Dangerous items.
2.  **Satire via Systems:** The horror of the setting is conveyed through "Corporate Benevolence" mechanics (Smile Checks) that interrupt flow.
3.  **The Boiling Frog:** The game starts normal and slowly descends into dystopia without the UI ever acknowledging the horror.
4.  **Rhythmic Flow:** Mastery of the typing rhythm provides the "fun" counterweight to the narrative "stress."

### Core Gameplay Loop

1.  **Select Order:** Identify priority orders on the computer screen.
2.  **Fulfill:** Type item names to box them.
3.  **Comply:** Respond to "Smile Checks" and "Morning Pledges" to avoid penalties.
4.  **Ship:** Complete orders before the TOT (Time Off Task) timer runs out.

### Win/Loss Conditions

### Win/Loss Conditions
*   **Unique Endings:**
    *   **Loyalist Ending (The Good Drone):** Survive Day 20 with High Stars / Low Resistance.
        *   *Result:* Promotion to Middle Management. You are now the one sending the oppressive emails. (Bleak/Cyclical).
    *   **Resistance Ending (The Spark):** Survive Day 20 with High Resistance Points.
        *   *Result:* The warehouse goes on strike. The system halts. You are arrested, but you see the lines stop moving. (Heroic/Sacrificial).
    *   **Failure:** Fired (0 Stars) or "Disappeared" (Max Demerits).

---

## Game Mechanics

### Primary Mechanics

*   **Complex Typing:** Type item names to box them. Modifiers (e.g., `[SHIFT] + [!]`) are added for hazardous/narrative items.
*   **Time Off Task (TOT):** A passive timer that punishes inactivity.
*   **Smile Verification:** A "Wellness Check" popup requiring specific inputs to "smile".
*   **Morning Pledge:** Daily slogan typing ritual.

### Controls and Input

{{controls}}

---

## Simulation Specific Elements

### Core Simulation Systems

**Workflow Simulation:** The game simulates a specific workstation workflow in high fidelity.
1.  **Computer (Order View):** Player accepts orders.
2.  **Warehouse (Request View):** Player types codes to request items from deep storage to the conveyor.
3.  **Boxing (Assembly View):**
    *   **Place:** Move items from conveyor to box.
    *   **Fold & Tape:** Execute typing sequences/gestures to seal the box.
    *   **Label:** Apply shipping data.
    *   **Ship:** Send it down the line.

**What's being simulated:**

- Primary simulation focus (city, farm, business, ecosystem, etc.)
- Simulation depth (abstract vs. realistic)
- System interconnections
- Emergent behaviors
- Simulation tickrate and performance

### Management Mechanics

*   **Resource Management:**
    *   **Emergent/Environmental:** The story is told primarily through the *Item Database* and *Email Notifications*.
    *   **Emails:** Context for rules (e.g., "Jon was caught discussing <party> -> New Pledge: 'No Politics'").
    *   **Corporate Directory:** An evolving org chart that tracks the staff's descent from corporate drones to militarized fanatics (or "missing" persons).
    *   **Time:** The primary constraint (TOT / Quota).
    *   **Inventory Space:** Limited conveyor/box space requires "Tetris-lite" management.
*   **Strategic Decisions:**
    *   **Order Selection (Conditional):** normally **Linear/Forced**.
    *   **Off-Books Orders (Resistance):** Optional "Glitch" orders that appear. Completing them earns **Black Market Points** but risks high TOT/Fines.
    *   **Black Market:** Spending Points to hack the system (lower quota) or buy comfort.

**Management systems:**

- Resource management (budget, materials, time)
- Decision-making mechanics
- Automation vs. manual control
- Delegation systems (if applicable)
- Efficiency optimization

### Building and Construction

*   **Core Decision:** **N/A (or Low Priority).**
*   **Rationale:** The game focuses on *player skill* (APM/Accuracy) rather than *system automation*. Buying "Auto-Tapers" would remove the core gameplay loop.
*   **Potential:** "Manual Tools" (e.g., a faster tape gun) could be investigated, but strictly no "Idle Game" automation.

**Construction systems:**

- Placeable objects/structures
- Grid system (free placement, snap-to-grid, tiles)
- Building prerequisites and unlocks
- Upgrade/demolition mechanics
- Space constraints and planning

### Economic and Resource Loops

*   **Resource A: Gold Stars (Corporate Standing)**
    *   **Nature:** Survival Metric (Health). High = Safe, Zero = Fired.
    *   **Source:** Perfect Orders, "Smiling", Daily Quota.
*   **Resource B: Black Market Points (Resistance Crypto)**
    *   **Nature:** Spendable Currency.
    *   **Source:** **Off-Books Orders** (Smuggling requests found in dark web or glitch orders).
    *   **Sink (Black Market):** Buying "Quota Hacks" (Easier day), "TOT Freezes", or localized disruptions.
*   **Loop:** Risk your Safety (Stars/Time) to do Resistance work -> Earn Points -> Buy Hacks to make the Corporate Job survivable.

**Economic design:**

- Income sources
- Expenses and maintenance
- Supply chains (if applicable)
- Market dynamics
- Economic balance and pacing

### Progression and Unlocks

*   **Campaign Structure:** 20 Days fixed.
*   **Metagame:** "Roguelite-ish" replayability. If you fail Day 18, you might need to restart to earn more stars in Day 1-10 to afford the bribes needed for Day 18.
*   **Unlocks:** New Item Tiers (Narrative progression), Black Market access (Day 5+).

**Progression systems:**

- Unlock conditions (achievements, milestones, levels)
- Tech/research tree
- New mechanics/features over time
- Difficulty scaling
- Endgame content

### Sandbox vs. Scenario
*   **Primary Mode:** **Scenario (Story Campaign).** A scripted 20-day narrative descent.
*   **Secondary Modes:** **N/A (Launch).** Sandbox or Endless modes are strictly POST-LAUNCH features to preserve scope.

**Game modes:**

- Scenario/Story Mode (Main focus)
- Sandbox/Endless (Out of Scope for V1)

---

## Progression and Balance

### Player Progression

### Player Progression
*   **Item Mastery:** Players learn complex item names (muscle memory).
*   **Account Upgrades:**
    *   **Gold Stars:** Earned currency.
    *   **Black Market:** Unlocked on Day 5. Allows purchase of "Bribes" (lower quota, slower TOT) or "Comforts" (music tracks).
*   **Narrative Unlocks:** New emails and "Item Tiers" unlock as days progress (Tier 1 Consumer -> Tier 3 War Supplies).

### Difficulty Curve
*   **Ramp:**
    *   **Day 1-4:** Training. Low quota, no modifiers.
    *   **Day 5-10:** "The Squeeze." TOT introduced. Smile Checks start.
    *   **Day 11-15:** "War Economy." Modifiers (Shift/Alt) appear on items. Quotas spike.
    *   **Day 16-20:** "Survival." High complexity, fail-state pressure. Requires Black Market usage to survive.
*   **Pacing:** Wave-based intensity within a single day (Rush Hour vs Lull).

### Economy and Resources
*   **Gold Stars:**
    *   **Earn:** **Lump Sum per Quota Complete** (e.g., +20 Stars). No per-order payout.
    *   **Logic:** Raising the quota makes the game harder (more work for the *same* pay - "Workplace Shrinkflation").
    *   **Spend:** -10 to Remove TOT Strike, -20 to Lower Next Day Quota.

---

## Level Design Framework

### Level Types

### Level Types
*   **The "Day":** Each level is one work shift (approx 5-8 minutes).
*   **Ritual Phase:** "Morning Pledge" minigame intro.
    *   **Progression:** Innocent ("Have a nice day") -> Industrial ("I am a boxing machine") -> Suppressive ("I promise not to bring my political affiliation to work") -> Cultish ("Our great leader can do no wrong").
*   **Work Phase:** The core sorting loop.
*   **Review Phase:** Email check and Black Market shopping.
    *   **Corporate Directory:** Optional screen to view the staff list.
    *   **Progression:** Starts as a normal org chart. Over time, it devolves into a chaotic web of militaristic portraits, "missing" stamps, and nonsensical connections, reflecting the company's descent into madness.

### Level Progression
*   **Linear Campaign:** Days 1 through 20.
*   **Events:** Specific scripted events happen on fixed days (e.g., "Day 6: CEO Announcement - Smiling is Mandatory").
*   **Failure:** Failing a day restarts that Day (with currently held Stars). Use "Roguelite" logic where money persists? (TBD: simpler to just checkpoint start of day).

---

## Art and Audio Direction

### Art Style

### Art Style
*   **Aesthetic:** **90s Corporate Hellscape ("Office Space" Aesthetic).**
*   **Palette:**
    *   **Standard:** Drab Beiges, Faded Greys, and washes of fluorescent white light. The "Soulless Cubicle" look.
    *   **UI:** Windows 95-era grey bevels, deep blue title bars, and clunky serif fonts.
    *   **Danger:** Passive-aggressive bright yellow sticky notes and "PC Load Letter" style system errors.
    *   **Corruption:** The desktop interface glitching not into static, but into "Blue Screens of Death" and melting icons.
*   **UI Focus:** Intentionally retro desktop interface (fake OS), balancing nostalgia with bureaucratic confinement.

### Audio and Music
*   **Dynamic Soundtrack:** Single "Corporate Bossa Nova" track.
    *   **State 1:** Clean, high fidelity.
    *   **State 2:** Slight detuning, vinyl crackle (Stress Low).
    *   **State 3:** Bit-crushed, skipped beats (Stress High).
    *   **State 4:** Harsh noise/drone overlay (Critical/War).
*   **SFX:**
    *   **Good:** Thocky mechanical production keyboard sounds.
    *   **Bad:** Harsh, jarring system buzzers.

---

## Technical Specifications

### Performance Requirements

### Performance Requirements
*   **60 FPS Minimum:** Input latency must be zero. Typing games live or die on responsiveness.
*   **Low Spec Friendly:** Should run on any potato laptop (integrated graphics).

### Platform-Specific Details
*   **PC:** Exclusive focus.
*   **Input:** Raw Keyboard Input handling (scan codes) needed to bypass OS repeat rates for pro typists.

### Asset Requirements
*   **Items:** ~100 Item Definitions (JSON). No 3D models needed (Text only).
*   **Audio:** 1 Adaptive Music System, ~20 UI SFX.
*   **Shaders:** CRT/Glitch shader stack.

---

## Development Epics

### Epic Structure

### Epic Structure
1.  **Core Fulfillment Loop (P0):** Order -> Conveyor -> Box -> Ship. (MVP Complete)
2.  **Oppression Systems (P0):** TOT Timer, Smile Verification, Daily Pledge.
3.  **Economy & Meta (P1):** Gold Stars, Black Market UI, Day Cycle logic.
4.  **Content & Narrative (P1):** Item DB (tiers), Email System, "Glitch" progression.
5.  **Juice & Polish (P2):** CRT Shaders, Camerashake, Audio Manager.

---

## Success Metrics

### Technical Metrics

### Technical Metrics
*   **Input Latency:** < 16ms.
*   **Crash Rate:** 0% (Stable C++ build).

### Gameplay Metrics
*   **WPM (Words Per Minute):** Track average player speed.
*   **Error Rate:** Target < 5% for average player.
*   **Day Completion Rate:** Identifying "quit moments" in the campaign.

---

## Out of Scope

*   **Multiplayer:** Explicitly single-player.
*   **3D Inventory Physics:** Items are abstract text/icons, not physics objects.
*   **Character Customization:** Player is a faceless drone.

---

## Assumptions and Dependencies

*   **User Workspace:** Access to `~/p/pharmasea` was restricted, so architecture follows `prime_pressure` existing patterns (ECS/Raylib).
*   **Music:** Assuming external assets or simple self-made tracks.
*   **Timeline:** 3-month cycle assumes focused solo dev speed.
\n---\n
## epics
# Prime Pressure - Development Epics

## Epic Overview

| ID | Title | Priority | Description |
|----|-------|----------|-------------|
| **E01** | **Core Fulfillment Loop** | P0 (MVP) | The baseline "Order -> Box -> Ship" gameplay. (Already mostly implemented). |
| **E02** | **Oppression Systems** | P0 (MVP) | The mechanics that apply pressure: TOT Timer, Smile Checks, Daily Pledges. |
| **E03** | **Economy & Meta** | P1 | The "Gold Stars" economy, Black Market shop, and Day/Run progression logic. |
| **E04** | **Content & Narrative** | P1 | The data-driven content: Item Tiers (1-3), Email System, and Glitch Progression. |
| **E05** | **Game Juice & Polish** | P2 | Visual/Audio polish: CRT Shaders, Camerashake, Dynamic Audio Manager. |

---

## Epic Breakdowns

### Epic 1: Core Fulfillment Loop (P0 - MVP)

**Goal:** Solidify the "happy path" of receiving, typing, boxing, and shipping an order.
**Status:** Mostly Implemented (Refining existing ECS).

#### Stories

**Story 1.1: Refine Boxing Input State**
*   **As a** Player,
*   **I want** the boxing phase to accept typed input clearly (Place -> Fold -> Tape -> Label),
*   **So that** I can complete the physical shipping process.
*   **Acceptance Criteria:**
    *   [ ] Pressing keys in sequence triggers boxing animations.
    *   [ ] "Tape" requires a specific multi-key sequence (e.g., Hold Space + Drag/Type).
    *   [ ] Completion triggers `OrderShipped` state.

**Story 1.2: Order Monitor UI**
*   **As a** Player,
*   **I want** to see the incoming stream of orders on the "Computer Screen",
*   **So that** I know what items are coming next.
*   **Acceptance Criteria:**
    *   [ ] Computer View lists current active order + next 3 in queue.
    *   [ ] Default State: **Locked/Linear** (Player cannot reject orders).
    *   [ ] Pressing `[TAB]` switches between Computer and Warehouse views.

---

### Epic 2: Oppression Systems (P0 - MVP)

**Goal:** Implement the "Pressure" mechanics that force the player to panic.

#### Stories

**Story 2.1: Time Off Task (TOT) Timer**
*   **As a** Manager (System),
*   **I want** to track player inactivity,
*   **So that** I can punish them for being slow.
*   **Acceptance Criteria:**
    *   [ ] Timer bar fills up when no input is detected for > 2 seconds.
    *   [ ] Filling the bar triggers a "Warning" SFX and visual overlay (Red Border).
    *   [ ] 3 Warnings = 1 Strike (Fine).

**Story 2.2: Smile Verification System**
*   **As a** System,
*   **I want** to interrupt the player with a "Smile Check",
*   **So that** I can ensure compliance and break their flow.
*   **Acceptance Criteria:**
    *   [ ] Randomly triggers during the `Work Phase`.
    *   [ ] Pop-up covers center screen: "Please Smile for Verification".
    *   [ ] Player must hold a key (e.g., `[Right Shift]`) for 1 second to clear.
    *   [ ] Failure to smile pauses all inputs and raises TOT.

**Story 2.3: Morning Pledge Minigame**
*   **As a** Company,
*   **I want** the player to type a loyalty pledge before starting the day,
*   **So that** they are indoctrinated.
*   **Acceptance Criteria:**
    *   [ ] New Game State: `DayStart_Pledge`.
    *   [ ] Text crawler shows the daily slogan.
    *   [ ] Player must type it 100% correctly to unlock the shift key.

---

### Epic 3: Economy & Meta (P1)

**Goal:** Create the resource loop (Earn Tokens -> Buy Survival).

#### Stories

**Story 3.1: Dual Economy System**
*   **As a** Designer,
*   **I want** two separate currencies (Gold Stars = Health, Market Points = Money),
*   **So that** players make tradeoffs between Safety and Power.
*   **Acceptance Criteria:**
    *   [ ] `GoldStars` persist across days. Hitting 0 = Game Over.
    *   [ ] `MarketPoints` earned *only* via "Glitch/Illegal" orders.
    *   [ ] UI shows Stars (Prominent) and Points (Hidden/Subtle).

**Story 3.2: Off-Books Order Injection**
*   **As a** Player,
*   **I want** to see occasional "Glitch Orders" (Resistance requests),
*   **So that** I can choose to take the risk.
*   **Acceptance Criteria:**
    *   [ ] `OrderGenerator` has 5% chance to spawn a `ResistanceOrder`.
    *   [ ] Resistance Orders have "Garbled" text until hovered.
    *   [ ] Completing them grants `MarketPoints` but risks `TOT` alerts.

**Story 3.3: Black Market Interface**
*   **As a** Player,
*   **I want** to spend Market Points on hacks,
*   **So that** I can survive the later days.
*   **Acceptance Criteria:**
    *   [ ] Shop uses `MarketPoints` currency.
    *   [ ] Items: "Clear Strikes" (Safety), "Slow Down Conveyor" (Ease).

**Story 3.3: Corporate Directory Screen**
*   **As a** Player,
*   **I want** to see the staff directory change over time,
*   **So that** I understand the narrative stakes (people disappearing).
*   **Acceptance Criteria:**
    *   [ ] New View: `DirectoryView`.
    *   [ ] Displays grid of portraits/names.
    *   [ ] `DayEventSystem` can tag specific NPCs as "Missing" or "Promoted" (Change portrait to skull/soldier).

---

### Epic 4: Content & Narrative (P1)

**Goal:** Populate the game with the "stuff" that tells the story.

#### Stories

**Story 4.1: Item Database & JSON Loader**
*   **As a** Developer,
*   **I want** to load items from a JSON file,
*   **So that** I can easily add hundreds of items/descriptions.
*   **Acceptance Criteria:**
    *   [ ] `ItemRegistry` loads `items.json`.
    *   [ ] Items have properties: `Name`, `Tier` (1-3), `FlavorText`, `Complexity` (Input string).

**Story 4.2: Modifier System**
*   **As a** Designer,
*   **I want** to attach special input rules to dangerous items,
*   **So that** they are harder to type.
*   **Acceptance Criteria:**
    *   [ ] Support for Modifiers: `HoldShift`, `DoubleTap`, `Reverse`.
    *   [ ] Visual indicator on the typing prompt showing the required modifier.

**Story 4.3: Email Narrative System**
*   **As a** Storyteller,
*   **I want** to push emails to the player's inbox between shifts,
*   **So that** I can deliver lore and foreshadowing.
*   **Acceptance Criteria:**
    *   [ ] `EmailManager` queues messages based on Day #.
    *   [ ] Unread badge on "Email" icon.
    *   [ ] "Forwarded" chains that show corporate conversations.

**Story 4.4: Narrative Evaluation & Endings**
*   **As a** Player,
*   **I want** my choices (Loyalist vs Resistance) to change the ending,
*   **So that** my struggle feels meaningful.
*   **Acceptance Criteria:**
    *   [ ] `EndingManager` checks stats on Day 20 completion.
    *   [ ] `ResistancePoints > Threshold` -> Trigger Revolution Ending (Strike Cinematic).
    *   [ ] `ResistancePoints < Threshold` -> Trigger Promotion Ending (Management Email).

---

### Epic 5: Juice & Polish (P2)

**Goal:** Make the dystopian misery feel "good" to play.

#### Stories

**Story 5.1: CRT Shader Stack**
*   **As a** Player,
*   **I want** the game to look like an old 90s monitor,
*   **So that** I feel immersed in the time period.
*   **Acceptance Criteria:**
    *   [ ] Implement Scanlines, Chromatic Aberration, and Curvature shaders.
    *   [ ] "Glitch" parameter that can be driven by Game Stress/Sanity.

**Story 5.2: Dynamic Audio Manager**
*   **As a** Player,
*   **I want** the music to distort when I am stressed,
*   **So that** the audio reinforces the gameplay tension.
*   **Acceptance Criteria:**
    *   [ ] Audio System supports 4 tracks (stems) or pitch-shifting.
    *   [ ] Links `TOT_Level` to `Distortion_Amount`.
    *   [ ] "Smile" success triggers a pleasant (fake) chime.

