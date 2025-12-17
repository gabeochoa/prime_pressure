# Prime Pressure — Implementation Doc (Intern Guide)

This is the **implementation guide** for building the next planned increment of Prime Pressure.

It is written for an intern who will be asked to “build the Day 1–3 slice.” It focuses on *what to implement*, *where to implement it*, and *how to prove it works*.

## Canonical references (read these first)

- **Architecture (authoritative):** `docs/architecture.md`
- **Requirements (authoritative):** `docs/prd.md`
- **Stories / acceptance criteria:** `docs/epics.md`
- **Project conventions:** `docs/project_context.md`, `PROJECT_RULES.md`
- **Current test direction (future work):** `docs/test-infrastructure-overhaul-plan.md`

If anything in this doc conflicts with `docs/architecture.md`, **follow `docs/architecture.md`**.

## What we are building (MVP)

### MVP target

Deliver a **Day 1–3 vertical slice** that proves the **Day Loop** is real and stable:

- **Pledge → Work → Review** phases each day
- Works for **Day 1, Day 2, Day 3**
- **Hard endpoint:** after the player acknowledges the End-of-Day 3 Review screen, the run reaches **“Day 3 Complete”** and stops advancing.

### Explicitly out of scope (do not implement)

From the PRD, these are intentionally **not** part of the Day 1–3 MVP:

- Gold Stars / Market Points economy
- Hazard materials / input modifiers
- Full menu UX
- Cutscenes / polish systems (CRT shaders, etc.)

## Non‑negotiables (don’t break these)

### Architecture invariants

- **Pure ECS:** state lives in components; logic lives in systems.
- **Sophie:** global/meta persistence is an **ECS singleton entity**, not a C++ singleton class.
  - **Prohibited:** `Sophie::get()` or any “global object” singleton pattern for Sophie.
- **Render systems are read‑only:** do not mutate gameplay state in rendering.
- **Selection invariants are hard-fail:** if code ever creates an invalid “multiple selected/active orders” state, log an error and crash. Do not silently auto-heal.

### Input invariants

- **Typing feel:** keep input responsive (perceived < 1 frame).
- **Gameplay key handling uses wrappers:** gameplay systems should read keys/chars via `game_input::...` (`src/input_wrapper.h`) so tests can inject input.
- **Shift encoding:** do not implement gameplay “shift checks” (like `IsKeyDown(KEY_LEFT_SHIFT)`) to interpret recipes/commands.
  - Shifted keys are represented in data/config via the `^` syntax (see `docs/architecture.md`, `docs/project_context.md`).
  - MVP decision: remove `Shift+TAB` view switching (no keyboard-Shift-based gameplay navigation).

## Where things live in this repo (orientation)

### Entry points

- Main entry point: `src/main.cpp`
- Game loop + system registration: `src/game.cpp`

### Components

- Most current components are aggregated in:
  - `src/components.h`
  - `src/order_components.h`

### Systems (update loop)

- Systems are currently header-based and registered in `src/game.cpp`.
- Look at these as examples of style and patterns:
  - `src/systems/ProcessTypingInputSystem.h`
  - `src/systems/ProcessBoxingInputSystem.h`
  - `src/systems/order_systems.h` (order selection/tabbing + workflow tag systems)

### Rendering

- Render systems are registered in `src/game.cpp` using helpers like:
  - `register_render_computer_systems(systems)` (`src/render_computer.cpp`)
  - `register_render_warehouse_systems(systems)` (`src/render_warehouse.cpp`)
  - `register_render_boxing_systems(systems)` (`src/render_boxing.cpp`)

### Tests

- Test runner is built into the game binary:
  - list tests: `./output/warehouse_game --list-tests`
  - run one: `./output/warehouse_game --run-test <name>`
- Current tests live under:
  - `src/testing/tests/`

## Implementation plan (do this in order)

This plan is designed to minimize risk: first create the state model, then wire the phase machine, then add pledge/review UI, then add pressure. Telemetry is explicitly deferred for now.

### Milestone 0 — Add Sophie + MVP meta components

**Goal:** create a single ECS entity that stores campaign/day state.

**Add components (MVP):**

- `CampaignProgress`
  - `int day_index` (1..3)
  - `enum struct DayPhase { Pledge, Work, Review, Complete }`
  - `bool slice_complete` (true at Day 3 completion; optional if you use `phase == Complete`)
- `WorkdayConfig`
  - fixed-length Work phase duration (MVP: Normal only)
  - quota target definition: **orders completed** (an order is counted when it reaches `OrderState::Complete_ClosedOut`)
- `WorkdayRuntime`
  - elapsed time in the Work phase for the current day
  - quota progress for the current day (count of orders completed this day)

**Where:**

- Add the new component structs in `src/components.h` (keep it minimal + POD).
- Create the Sophie entity in `src/game.cpp` near the other singleton entity creation.
  - Recommended: tag it by component presence (e.g., it’s “Sophie” if it has `CampaignProgress`).
  - Use `EntityQuery().whereHasComponent<CampaignProgress>().gen_first()` whenever you need it.

**Definition of done:**

- The game boots with Day 1 initialized and phase set to `Pledge`.
- There is exactly one Sophie entity.

### Milestone 1 — Implement the Day Loop state machine

**Goal:** implement the phase transitions and gating rules.

**State machine:**

- Start run → Day 1 → `Pledge`
- Pledge complete → `Work` and record `day_start(day_index)`
- Work complete → `Review` and record `day_end(day_index)`
- Review acknowledge:
  - if day_index < 3: increment day_index, go to `Pledge`
  - if day_index == 3: go to `Complete` and record `run_completed(day=3)`

**Where:**

- Add a new update system like `MetaDayLoopSystem` under `src/systems/` and register it early in the update list in `src/game.cpp`.

**Phase gating:**

- Gameplay systems that mutate fulfillment state must only run in `Work`.
- Pledge systems run only in `Pledge`.
- Review systems run only in `Review`.

**Practical implementation options (pick one and stick to it):**

- **Option A (preferred for clarity):** each “work gameplay” system checks Sophie phase at the start and early-returns unless phase is `Work`.
- **Option B:** create one “gate” system that writes a singleton `GameplayPhaseGate` component (`allow_work_systems` bool), then have work systems check that instead.

**Definition of done:**

- You can advance through Day 1 → Day 2 → Day 3 and reach a stable “Day 3 Complete” endpoint.
- Transitions do not accidentally consume buffered typing.

### Milestone 1.25 — Workday timer + quota success/failure

**Goal:** define what makes a Workday succeed.

**Workday start/end UX (MVP):**

- After pledge completion, fade to black and then fade into the normal gameplay UI for the Work phase.
- Treat a “morning bell” moment as the start of Workday timekeeping (start the Workday timer there).
- Show a clock on the computer UI:
  - starts at **9:00 AM**
  - reaches **5:00 PM** when the Workday ends
- When the Workday ends, fade to black and then fade into the computer email view (Review phase).

**Timer:** Workday duration is fixed per difficulty and maps to the 9am→5pm clock span for presentation.
  - MVP decision: ship **Normal only** for now.

**Quota:** a Workday is successful only if the player completes enough orders before the timer expires.

- A completed order is one that reaches `OrderState::Complete_ClosedOut`.
- Workday quota progress is reset at the start of each Workday.

**Implementation sketch:**

- `WorkdayTimerSystem` (update): increments `WorkdayRuntime.elapsed_seconds` only during `phase == Work`.
- `WorkdayQuotaSystem` (update): counts orders in `Complete_ClosedOut` that were completed “this day” and updates `WorkdayRuntime.quota_progress`.
  - MVP note: if you don’t have per-day completion stamps yet, track a `counted_completed_orders` set/list for the current day so you only count each order once.
- `WorkdayCompletionSystem` (update): when timer expires:
  - if quota met: transition to Review
  - else: fail the Workday and transition to Review (summary email should be a “you’re fired” message)
    - MVP behavior: when the player closes the “you’re fired” email, quit the game.
    - TODO: define the real failure flow (retry day, restart run, penalties, etc.).
    - Temporary guard: `log_error("handle failure state")` on close until the flow is fully implemented.

### Milestone 1.5 — Convert selected/active order from singleton IDs to tags

**Goal:** make “what order is selected” queryable and entity-native.

**Decision (MVP):** selected and active are the same concept. Selection is tracked on order entities via a tag, not via singleton components that store `EntityID`.

**Tags:**

- `GameTag::IsSelectedOrder`

**Invariant:** there must be **at most one** selected order and **at most one** active order at a time.

**Invariant (MVP):** there must be **at most one** selected order at a time.

- If more than one is detected, `log_error(...)` and crash (intentional).

**Where:**

- Update `src/systems/ProcessOrderSelectionSystem` and `src/systems/ProcessOrderTabbingSystem` to set/clear `GameTag::IsSelectedOrder` on the order entities.
- Remove the singleton component `SelectedOrder` (and its creation/registration in `src/game.cpp`).
- Remove the singleton component `ActiveOrder` (MVP: not needed since selected == active).
- Update any systems/renderers that reference `SelectedOrder` / `ActiveOrder` to use the selected-order tag instead.

**Definition of done:**

- Selecting/tabbing orders works as before.
- No `SelectedOrder` / `ActiveOrder` singletons remain.
- Render highlights are driven by tags.

### Milestone 2 — Morning pledge (Pledge phase)

**Goal:** implement the pledge typing mini-screen.

**Behavior:**

- When `CampaignProgress.phase == Pledge`, fade in from black with a simple “DAY X” title card, then show the pledge prompt.
- Player types the pledge text.
- There is a timer, but it should be easy to hit (target ~40 WPM).
- Pass conditions (MVP):
  - If the pledge is finished, it passes.
  - If the timer expires, it still passes if the player is at least ~80% complete.
- If the timer expires and the player is < 80% complete:
  - TODO: define the failure/punishment behavior (what happens next).
- Bonus (TODO): faster typing + better accuracy yields a “bonus gold star” (economy is out of scope; log/placeholder only).

**Implementation notes:**

- Keep pledge input separate from the existing “work typing buffer” so Work-phase buffers can’t accidentally dismiss the pledge.
- On entering pledge, ensure work gameplay inputs are ignored.
- Reuse the same pledge text for Days 1–3 for now. (TODO: add story-specific pledges per day later.)
  - TODO: later, corporate tone early and more surreal/dystopian tone later as the campaign progresses.
  - On completion, show a small results view: WPM and a placeholder reward line (e.g., “+1 gold star”).
  - Then fade to black and transition into Work (Workday begins at the “morning bell” moment).

**Where:**

- Add components for pledge UI/input state (e.g., `PledgeState` on Sophie or on a pledge UI entity).
- Add `ProcessPledgeInputSystem` (update) and `RenderPledgeSystem` (render).

**Definition of done:**

- Pledge phase reliably blocks Work systems.
- The pledge can be completed on Days 1–3.

### Milestone 3 — End-of-day review/email (Review phase)

**Goal:** implement an end-of-day “inbox” where the player must open the summary email to proceed.

**Behavior:**

- Emails are ECS entities with an `Email` component: `{from, header, body, unread}`.
- Inbox selection/view state is stored in ECS components (recommended: on the Sophie entity).
- The summary email exists for each day and is marked unread on entry to Review.
- In addition to the summary, add **1–3 additional emails** (random count) on entry to Review (MVP content can be placeholder `header_text` / `body_text`).
- The player must open the summary email at least once to be allowed to end the day.
- Controls (MVP):
  - arrow keys: navigate email selection (does not open)
  - `Enter`: open the selected email in a viewer
  - `Esc` or `X`: close email viewer
  - `Tab`: end day (only after summary opened)
    - TBD: if the viewer is open, either close it first or end day; decide later.
  - MVP decision: view switching/navigation should not rely on keyboard Shift.

**Implementation notes:**

- On entering Review, flush/ignore leftover Work-phase input so stray keystrokes don’t auto-advance.
- Separate logic from rendering:
  - update systems mutate `Email`/selection/viewer components
  - render systems only draw the inbox + selected email view
 - Summary email content:
   - Workday success: corporate summary (placeholder header/body for now)
   - Workday failure: “you’re fired” summary (placeholder header/body for now)
   - TODO: corporate tone early; more surreal/dystopian tone later as the campaign progresses

**Recommended query shape (inbox):**

- Unread list: `EntityQuery().whereHasComponent<Email>().whereLambda(...unread...).gen()`

**Where:**

- Add a helper to create an email entity (function or small builder) such as `make_email(...)`.
- Add `ProcessReviewInputSystem` (update) and `RenderReviewSystem` (render). If you want, split render into `RenderInboxSystem` + `RenderEmailViewerSystem`.

**Definition of done:**

- Review always appears after the Work completion trigger.
- The Day 3 Review can be acknowledged and results in a stable endpoint.

### Milestone 4 — Oppression systems (deferred)

TOT (Time Off Task) and Smile Verification are deferred for now. (TODO: revisit once the Day Loop + Workday quota pacing feels right.)

### Milestone 5 — Telemetry/events (minimum viable)

Telemetry is deferred for now.

### Milestone 6 — Tests (minimum coverage)

**Goal:** add at least one test that proves you can progress through the Day Loop without softlocking.

**How tests work today:**

- Tests run inside the real game loop via `TestSystem`.
- Input is injected through `test_input::input_queue` and consumed by `game_input::IsKeyPressed` / `game_input::GetCharPressed`.

**Add tests:**

- `TestDayLoopAdvancesDays1to3`:
  - complete pledge
  - do whatever is required to finish Work for a day
  - acknowledge Review
  - repeat until Day 3 Complete

If completing Work requires a lot of gameplay steps, add a **temporary MVP-only “debug end shift”** input (Work-only) that ends the day; remove it once real shift completion is defined.

## Build / run / test commands

- Build: `make`
- Run: `make run`
- Run tests: `make test` (or `scripts/run_all_tests.sh`)
- List in-game tests: `./output/warehouse_game --list-tests`

## Common failure modes (and how to avoid them)

- **Softlock on phase transitions:** always clear or isolate input buffers when moving between phases.
- **Work systems running in pledge/review:** enforce gating explicitly (don’t rely on “it won’t happen”).
- **Multiple selected/active orders:** treat as a fatal invariant violation (log_error + crash) so it is fixed immediately.
- **Multiple selected orders:** treat as a fatal invariant violation (log_error + crash) so it is fixed immediately.
- **Tests can’t drive input:** ensure gameplay systems use `game_input::...` wrappers.
- **Day 3 completion doesn’t count:** “completion” requires reaching End-of-Day 3 and advancing past it to the “Day 3 Complete” endpoint.
- **Shift-based navigation:** MVP removes `Shift+TAB`; do not add new Shift-based gameplay navigation.

## Definition of Done (intern checklist)

The Day 1–3 slice is “done” when:

- [ ] Day 1 starts in `Pledge`
- [ ] Pledge → Work → Review transitions work for Day 1, Day 2, Day 3
- [ ] Day 3 Review advances to a stable “Day 3 Complete” endpoint
- [ ] Work-only systems do not mutate state during Pledge/Review
- [ ] At least one E2E test proves Days 1–3 advancement without softlock

## Boxing plan (refactor guideline)

Boxing step state should live **only** in `OrderWorkflow.state`. Do not maintain a second boxing state machine.

### What the boxing system needs to track

The only boxing “extra state” we need on the order (beyond `OrderWorkflow.state`) is progress inside the `Boxing_PutItems` step:

- `OrderBoxingRuntime { int items_placed = 0; }`

Total items to place can be derived (MVP choice):

- `total_items = order.ready_items.size()`

### How it should work

- If the active order is in boxing macro state:
  - `B` advances `OrderWorkflow.state` from `Boxing_FoldBox` → `Boxing_PutItems` and resets `items_placed = 0`.
  - `P` in `Boxing_PutItems` increments `items_placed`. (MVP note: there is no incorrect placement today.) When `items_placed >= total_items`, advance to `Boxing_Fold`.
  - `F` advances to `Boxing_Tape`.
  - `T` advances to `Boxing_Ship`.
  - `S` finalizes shipping and advances the workflow to stamping (`Shipped_Stamp0`).

### What to remove

- Remove the singleton `BoxingProgress` and the duplicated `BoxingState` state machine tracking.
- Do not spawn per-item boxing entities unless/until you need per-item UI; keep MVP as a simple counter.
