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
