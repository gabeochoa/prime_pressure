# Project Context (Prime Pressure)
---

## Prime Pressure (Canonical)

This document is the **single source of truth** for Prime Pressure conventions.

- **Authoritative architecture**: `docs/architecture.md`
- **Authoritative coding rules**: `PROJECT_RULES.md`
- **Reference-only (non-authoritative)**: Kart Chaos / Pharmasea notes below are included only as examples of prior preferences and familiarity.

### Critical Rules

#### Architecture & ECS
- **Core library**: Use the `afterhours` vendor library for ECS functionality.
- **Fluent queries**: Use `EntityQuery` (or `EQ()`) with fluent syntax for ad-hoc entity retrieval.
  - Example: `EntityQuery().whereHasComponent<T>().whereLambda(...).gen_first()`
- **Sophie persistence (Prime Pressure invariant)**: Sophie is an **ECS singleton entity** for global/meta-game state.
  - Do **not** implement or use a `Sophie::get()` C++ singleton in Prime Pressure.
- **Functionality-first rendering**: Rendering logic is split into focused render systems, not a monolithic render loop.

#### Coding Standards
- **Standard**: C++23 is strictly enforced.
- **Naming**:
  - **Variables/Functions**: `snake_case` (e.g., `get_spawn_position`, `running`)
  - **Types/Classes/Components**: `PascalCase` (e.g., `SystemManager`, `Transform`)
- **Ownership**: Use `std::unique_ptr` for system registration and ownership transfer. Avoid raw pointers for ownership.

#### Input Handling
- **Single Keystroke JSON syntax**: Input configuration uses a specific JSON syntax where `^` denotes the **keyboard Shift** modifier.
  - Example: `^P` represents `keyboard Shift + P`.
  - Do NOT use `"Shift+P"` or other variations.
- **No keyboard Shift checks in gameplay**: Do NOT check for `IsKeyDown(KEY_LEFT_SHIFT)` (or similar) in gameplay code for command inputs. Rely on the input mapping system and the `^` syntax.

---

## Appendix: Reference contexts (non-authoritative)

### Kart Chaos (reference only)

#### Notes
These are reference conventions from Kart Chaos and are **not** authoritative for Prime Pressure.

#### Critical Rules (Kart Chaos)

##### Architecture & ECS
- **Core Library**: Kart Chaos used the `afterhours` vendor library for ECS functionality.
- **Fluent Queries**: Use `EntityQuery` (or `EQ()`) with fluent syntax for ad-hoc entity retrieval.
    - Example: `EntityQuery().whereHasComponent<T>().whereLambda(...).gen_first()`
- **"Sophie" Singleton**: Kart Chaos used the "Sophie" pattern for global/meta-game state persistence, accessed via `EntityHelper::get_singleton_cmp<T>()`.
- **Functionality-First Rendering**: Rendering logic is split into specific systems (e.g., `RenderSpritesWithShaders`, `RenderEntities`) rather than a monolithic render loop.

##### Coding Standards
- **Standard**: C++23 is strictly enforced.
- **Naming Conventions**:
    - **Variables/Functions**: `snake_case` (e.g., `get_spawn_position`, `running`).
    - **Types/Classes/Components**: `PascalCase` (e.g., `SystemManager`, `Transform`, `EntityAndRegistry`).
    - **Constants**: `snake_case` or `kCamelCase` (mixed usage, prefer consistent `snake_case` for new constants).
- **Ownership**: Use `std::unique_ptr` for system registration and ownership transfer. Avoid raw pointers for ownership.

##### Input Handling
- **Single Keystroke Syntax**: Input configuration uses a specific JSON syntax where `^` denotes the **keyboard Shift** modifier.
    - Example: `^P` represents `keyboard Shift + P`.
    - Do NOT use "Shift+P" or other variations.

#### Implementation Patterns

##### Component Definition
Components must inherit from `::afterhours::BaseComponent` and should be primarily Plain Old Data (POD) structs.
```cpp
struct Transform : ::afterhours::BaseComponent {
    vec2 position{0.f, 0.f};
    vec2 velocity{0.f, 0.f};
    // Helper methods allowed for data manipulation
    vec2 center() const { ... }
};
```

##### System Definition
Systems inherit from `afterhours::System<Components...>` to define required components for iteration.
```cpp
struct MoveSystem : afterhours::System<Transform, Velocity> {
    virtual void for_each_with(Entity &entity, Transform &t, Velocity &v, float dt) override {
        // Logic here
    }
};
```

##### Rendering Pipeline
The rendering pipeline is multi-pass, utilizing `RenderTexture2D`:
1.  **World Render**: Entities, map elements.
2.  **Tag Shader**: Special effects (e.g., spotlight).
3.  **Post Processing**: Global screenspace effects (CRT, chromatic aberration).
4.  **UI**: HUD and debug overlays on top.

##### Input Mapping
Inputs are mapped to abstract `InputAction` enums rather than checking keys directly in game logic.
```cpp
if (action_matches(input.action, InputAction::Boost)) { ... }
```

#### Anti-Patterns (What NOT to do)

- **Native keyboard Shift Checks**: Do NOT check for `IsKeyDown(KEY_LEFT_SHIFT)` directly in gameplay code for command inputs. Rely on the input mapping system and the `^` syntax.
- **Monolithic Systems**: Avoid creating "God Systems" that do everything. Break down logic into small, focused systems (e.g., `UpdateSpriteTransform`, `UpdateAnimationTransform`).
- **Manual Memory Management**: Avoid `new`/`delete`. Use smart pointers and containers (`std::vector`, `std::optional`).
- **Ignoring C++23 Features**: Do not use outdated C++ patterns when C++23 alternatives (like `std::ranges`, `std::format` via `fmt`, etc.) are available.



### Pharmasea (reference only)

Note: Pharmasea used a C++ singleton access pattern (`Sophie::get()`). Prime Pressure does **not**; Prime Pressure uses an **ECS singleton entity** for Sophie instead.

#### Project Context (Pharmasea reference)

#### Critical Rules (Pharmasea)

- **ECS Patterns**
  - Use the **afterhours** library for all entity management. All queries must be performed via the fluent `EntityQuery` API (see `src/entity_query.h`).
  - In Pharmasea, **Sophie** was a C++ singleton accessed via `Sophie::get()` (see `src/system/sophie.cpp`). Never store raw pointers to the singleton.
  - Components must be **PascalCase** structs inheriting from `BaseComponent` (see `src/components/*.h`).
  - Systems operate on `RefEntity`/`OptEntity` returned by `EntityQuery` and must not manually iterate over raw containers.

- **Coding Style**
  - Variable and function names use **snake_case** (e.g., `render_floating_name`).
  - Component and type names use **PascalCase** (e.g., `HasSubtype`, `CanBeHeld`).
  - All headers start with `#pragma once` and include guards are not used.
  - Prefer early returns for error handling; avoid deep nesting.
  - Do not use `auto` for non‑template types – write explicit types.

- **Input Handling**
  - Input is defined using the **Single Keystroke JSON** syntax. Example: `{"key":"^P"}` represents **keyboard Shift‑P**. The caret (`^`) denotes the keyboard Shift modifier.
  - **Never** encode keyboard Shift modifiers directly in code; always use the JSON representation and let the input system translate it.

- **Constraints**
  - **No `keyboard Shift` modifiers** may appear in source code – they must be expressed only via the JSON syntax.
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
  This JSON is parsed by the input system and translated to a keyboard Shift‑P keystroke.

## Anti‑Patterns (What NOT to do)

- **Directly accessing keyboard Shift modifiers in code** – e.g., `if (IsKeyPressed(KEY_LEFT_SHIFT))` is prohibited.
- **Manual iteration over entity containers** – always use `EntityQuery`.
- **Using `auto` for concrete types** – write the full type name.
- **Mixing naming conventions** – do not use `camelCase` for variables or `snake_case` for components.
- **Hard‑coding singleton pointers** – never store raw `Sophie*`.
- **Using deprecated C++20 features** – avoid `std::bind`, raw `new/delete`, and `std::format`.
- **Placing system logic in UI code** – keep UI handling in the input system, not in rendering or game‑logic systems.

---
*Generated from analysis of `src/entity_query.h`, `src/system/sophie.cpp`, component headers, and project conventions in `PROJECT_RULES.md`.*

---

## Linked Documents

This file intentionally does **not** embed the full contents of other docs (to avoid drift). Use these instead:

- **Canonical architecture**: `docs/architecture.md`
- **Reference architecture**: `docs/reference_architecture.md`
- **Design**: `docs/GDD.md`, `docs/epics.md`, `docs/ux-design-specification.md`
