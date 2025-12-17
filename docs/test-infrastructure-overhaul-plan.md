# Test Infrastructure Overhaul Plan (Forward-Only State System)

## Goals
- **E2E/Jest-style tests** that run the real game loop and real systems (no gameplay changes).
- Tests should only need a small set of primitives:
  - `validate_current_state()`
  - `submit_input(...)`
  - `wait_until(...)` / `wait_for(EQ...)`
- Reduce flakiness by leveraging the fact that **states only move forward**.
- Make tests easy to run locally and in CI.

## Current State (What Exists Today)
- **Runner**: `./output/warehouse_game --run-test <name>` runs one coroutine test inside the game loop via `TestSystem`.
- **Input injection**:
  - Tests push events into `test_input::input_queue`.
  - Gameplay reads input via `game_input::{IsKeyPressed, GetCharPressed}` (`src/input_wrapper.h`). In `test_mode`, it is deterministic and does not fall back to real input.
  - Input consumption is limited to **one key + one char per frame**, with flags reset in `TestFrameBeginSystem`.
- **Waits**: `TestApp::wait_for_condition(...)` is frame-based.
- **Gap**: There is a `state_trace` concept, but nothing records transitions; transient states are easy to miss with equality checks.

## Target Design (What We Want)
### 1) A Minimal E2E Driver API
Create a new test-facing API (tests should use this, not internal helpers):

#### `validate_current_state()`
Validates invariants for every `OrderWorkflow` entity:
- `workflow.state` exists in `STATE_TABLE`.
- Exactly **one** microstate tag and **one** macrostate tag are enabled.
- Enabled tags match `microstate_tag_of(workflow.state)` and `macrostate_tag_of(macro_state_of(workflow.state))`.
- **Forward-only**: state rank never decreases for a given `order_id` across frames.

#### `submit_input(...)`
Coroutine-friendly, self-pacing input helpers:
- `co_await submit_key(raylib_key)`
- `co_await submit_char(char)`
- `co_await submit_text("...")` (one char per frame)

Key behavior:
- **Self pacing**: after pushing input, wait until it is consumed (or queue decreases), instead of sprinkling `wait_for_frames(2)`.
- Clear error messages on timeout (what key/char was submitted, current queue state).

#### `wait_until(...)` / `wait_for(EQ...)`
Core wait primitive:
- `co_await wait_until("label", predicate, timeout=auto)`

Convenience wrappers:
- `wait_for_exists(EQ().where...)`
- `wait_for_count(EQ().where..., n)`

State-aware waits (avoid missing transient microstates):
- `wait_until_state_at_least(order_id, OrderState target)` (rank-based)
- `wait_until_macro_at_least(order_id, OrderMacroState target)` (rank-based)

### 2) Test-Only Runtime + Observer (Diagnostics + Monotonic Tracking)
Add a test-only runtime store and an observer system:

#### `TestRuntime` (new, test-only)
Tracks:
- `frame_index`
- `last_state_rank[order_id]`
- Optional ring buffer of the last N snapshots for good timeout dumps

#### `TestObserverSystem` (new, test-only)
Runs only when `test_input::test_mode` is enabled.
Each frame:
- Snapshot `OrderWorkflow` entities: `(order_id, state, macro, kind, time_in_state, tag counts)`
- Update monotonic `last_state_rank`
- Optionally call `validate_current_state()` every frame so failures are pinpointed to the exact frame.

This replaces fragile “did we see state X” logic with monotonic rank progression and better debug output.

### 3) Deterministic Simulation Time During Tests
In the test runner loop (`run_test(...)`):
- Use a **fixed dt** (e.g. `1/60.f`) rather than `raylib::GetFrameTime()`.
- Keep `--slow-test` as a visual sleep only.

Why:
- Avoid VM/GPU timing variability.
- Make `STATE_TABLE.min_duration_seconds` convertible into stable frame budgets.

### 4) Timeouts: Prefer Auto Defaults Derived from the State Machine
Introduce a `Timeout` helper:
- `Timeout::frames(n)`
- `Timeout::seconds(s)` (converted via fixed dt)
- `Timeout::auto_for_state_transition(from, to)`:
  - sum `STATE_TABLE[*].min_duration_seconds` across the rank path + slack
  - convert to frames

Defaults:
- Query waits: conservative fixed default (e.g. 2–5 simulated seconds).
- State waits: derived from `STATE_TABLE` + slack.

### 5) Ensure All Gameplay Input Uses `game_input` Wrappers
Audit systems and replace any direct `raylib::IsKeyPressed` / `raylib::GetCharPressed` calls that affect gameplay flow with `game_input::...`.

This guarantees tests can drive the game solely via `test_input::input_queue`.

## Implementation Steps (Suggested Order)
1. **Add driver API**: `src/testing/test_driver.h` (and `.cpp` if desired).
2. **Add runtime + observer**: `src/testing/test_runtime.h`, `src/systems/TestObserverSystem.h`.
3. **Wire observer + fixed dt** in `run_test(...)` in `src/game.cpp`.
4. **Improve TestSystem failure output**:
   - include last snapshots (orders/states), input queue size, active/selected order ids.
5. **Input audit**: ensure gameplay systems use `game_input` wrappers consistently.
6. **Migrate tests** (`src/testing/tests/*.h`) to:
   - use `submit_*` pacing instead of `wait_for_frames`
   - use `wait_until_state_at_least` rather than exact-state equality where transient
   - call `validate_current_state()` strategically (or rely on per-frame observer validation)
7. **Ergonomics**:
   - add `make test` target (build + run `scripts/run_all_tests.sh`)
   - optionally add `--run-all-tests`

## Example Test Shape (Target Style)
- `wait_for_count(EQ().where<OrderSlot>().where<Order>(), 3)`
- `submit_key(KEY_ONE)`
- `id = selected_order_id()`
- `wait_until_state_at_least(id, OrderState::Boxing_FoldBox)`
- `submit_key(KEY_B)`
- `submit_key(KEY_P)` repeated, then `F`, `T`, `S`
- `wait_until_macro_at_least(id, OrderMacroState::Shipped)`
- `submit_key(KEY_R)`, `submit_key(KEY_T)`, `submit_key(KEY_S)`
- `wait_until_state_at_least(id, OrderState::Complete_ClosedOut)`
- `validate_order_completed_or_cleanedup(id)`

## Deliverables / Definition of Done
- Tests use only: `validate_current_state`, `submit_input`, `wait_until/wait_for`.
- No brittle frame-sleep choreography (minimal or none).
- Deterministic test mode (fixed dt) with state-aware default timeouts.
- Forward-only invariant enforced (monotonic rank) and tag invariants validated.
- Clear timeout dumps: current order states + last N snapshots + input queue info.
- One-command run path via `make test` (or existing script + binary flags).
