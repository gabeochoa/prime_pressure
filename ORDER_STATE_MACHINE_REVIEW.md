## Order system state machine review + implementation plan (intern-ready)

### Scope / constraints
- **No code changes in this review**. This file is a spec/plan for a future implementation.
- **Orders move forward only**.
- It must be obvious:
  - **What state** an order is in.
  - **What items** are in the order / what’s left.
- **Rendering should be easy**: renderers must be able to query/filter orders by ECS tags derived from the current order state.
- **Background processing**: orders should continue to process/advance while the player works other orders.

---

## Story beats (macro timeline)
These are the **major blocks** shown on the timeline:

1) Customer places order *(pre-game; becomes “arrived”)*
2) Order comes in
3) You open an order
4) Start requesting items from the warehouse
5) Items are received from the warehouse
6) Items are ready to be boxed
7) Items are boxed
8) Items are shipped (READY/TO/SHIP confirmation)
9) Order completes

We’ll keep these as **macro milestones** and represent “in-between” progress with **microstates**.

---

## Core design: microstates + macro milestones + tags

### State kinds
Every microstate is one of:
- **Input**: requires explicit player action to advance.
  - UI: **flash** (when selected) because it’s waiting on the player.
  - Tick: does *not* auto-advance.
- **Processing**: does not require input; advances automatically via time and/or world conditions.
  - UI: no flash; show “in progress”.
  - Tick: can auto-advance.
- **Terminal**: completed; never advances.

### Flashing policy
**Flashing is not a state.**
- Flash whenever the order is **selected** and the current microstate is **Input**.

### Why microstates help
- “Next step” is always obvious: each state has an explicit `next`.
- Renderers can query ECS tags like `OrderState::Receiving_OnConveyorMoving`.
- Debugging is easier: you can print one state name per order.

---

## The state model (spec)

### Macro milestones (what the timeline shows)
The macro timeline nodes are:

- `Incoming`
- `Opened`
- `RequestingItems`
- `ReceivingItems`
- `ReadyToBox`
- `Boxing`
- `Shipped`
- `Complete`

### Enum-style definitions (copy/paste friendly)
Use these as the canonical names for enums/tags.

#### `OrderMacroState` (timeline blocks)
```cpp
enum class OrderMacroState {
  Incoming,
  Opened,
  RequestingItems,
  ReceivingItems,
  ReadyToBox,
  Boxing,
  Shipped,
  Complete,
};
```

#### `OrderState` (microstates; one active per order)
```cpp
enum class OrderState {
  // Incoming
  Incoming_Arrived,
  Incoming_Backlogged, // optional/future

  // Opened
  Opened_Active,
  Opened_Inactive, // optional/future

  // RequestingItems
  Requesting_NeedsInput,
  Requesting_InputError,
  Requesting_AllRequested,

  // ReceivingItems (processing/dash steps; loops until AllReceived)
  Receiving_OnConveyorWaiting,
  Receiving_OnConveyorMoving,
  Receiving_ReceivedToReady,
  Receiving_AllReceived,

  // ReadyToBox
  ReadyToBox_Waiting,
  ReadyToBox_Queued, // optional/future

  // Boxing
  Boxing_FoldBox,
  Boxing_PutItems,
  Boxing_Fold,
  Boxing_Tape,
  Boxing_Ship,

  // Shipped (READY/TO/SHIP confirmation)
  Shipped_Stamp0,
  Shipped_Stamp1,
  Shipped_Stamp2,
  Shipped_Stamp3,

  // Complete
  Complete_CloseoutDelay,
  Complete_ClosedOut,
};
```

#### `OrderStateKind` (input vs processing vs terminal)
```cpp
enum class OrderStateKind {
  Input,
  Processing,
  Terminal,
};
```

### Microstate list (ordered, forward-only)
Each line is **State (kind) → next**.

If a Processing state needs to choose between two “next” states (common for loop/terminal detection), that choice must be made **inside the Processing completion handler** (see state table schema below). The state name still stays explicit and debuggable.

#### Incoming
- `Incoming_Arrived` (**Input**) → `Opened_Active`

*(Optional future:)*
- `Incoming_Backlogged` (**Processing**) → `Incoming_Arrived`

#### Opened
- `Opened_Active` (**Input**) → `Requesting_NeedsInput`

*(Optional future if you want it explicit; not required for correctness:)*
- `Opened_Inactive` (**Processing**) → `Opened_Active`

#### RequestingItems
- `Requesting_NeedsInput` (**Input**) → `Requesting_InputError` *(on invalid input)* **or** → `Requesting_AllRequested`
- `Requesting_InputError` (**Processing**, short cooldown/feedback) → `Requesting_NeedsInput`
- `Requesting_AllRequested` (**Processing**, short handoff) → `Receiving_OnConveyorWaiting`

#### ReceivingItems (these are the “dash” steps between request and ready-to-box)
- `Receiving_OnConveyorWaiting` (**Processing**) → `Receiving_OnConveyorMoving`
- `Receiving_OnConveyorMoving` (**Processing**) → `Receiving_ReceivedToReady`
- `Receiving_ReceivedToReady` (**Processing**) → `Receiving_OnConveyorWaiting` *(if more items remain)* **or** → `Receiving_AllReceived` *(if last item received)*
- `Receiving_AllReceived` (**Processing**, short handoff) → `ReadyToBox_Waiting`

Rule: enter `Receiving_AllReceived` **exactly once**, when the last required item is received.

#### ReadyToBox
- `ReadyToBox_Waiting` (**Input**) → `Boxing_FoldBox` *(or `Boxing_PutItems` if you skip fold-box)*

*(Optional future:)*
- `ReadyToBox_Queued` (**Processing**) → `ReadyToBox_Waiting`

#### Boxing (explicit step FSM)
- `Boxing_FoldBox` (**Input**) → `Boxing_PutItems`
- `Boxing_PutItems` (**Input**) → `Boxing_Fold`
- `Boxing_Fold` (**Input**) → `Boxing_Tape`
- `Boxing_Tape` (**Input**) → `Boxing_Ship`
- `Boxing_Ship` (**Input**) → `Shipped_Stamp0`

#### Shipped (READY/TO/SHIP confirmation microsteps)
- `Shipped_Stamp0` (**Input**) → `Shipped_Stamp1`
- `Shipped_Stamp1` (**Input**) → `Shipped_Stamp2`
- `Shipped_Stamp2` (**Input**) → `Shipped_Stamp3`
- `Shipped_Stamp3` (**Input**) → `Complete_CloseoutDelay`

#### Complete
- `Complete_CloseoutDelay` (**Processing**, **1.0s**) → `Complete_ClosedOut`
- `Complete_ClosedOut` (**Terminal**) → *(none)*

---

## Timeline rendering: major blocks + minor dashes
You want a timeline like:

`[ MAJOR ] - - - [ MAJOR ] - - [ MAJOR ] ...`

### Mapping rules
- **Major blocks** = the macro milestones above.
- **Dashes** = Processing microstates that sit *between* two macro milestones.
- Dash count is **variable** by design: if you add/remove Processing microsteps, the segment’s dash count changes automatically.

### Required state metadata (for UI + debugging)
For each microstate, define these metadata values in a single table:
- **macro**: which macro milestone it belongs to.
- **kind**: Input / Processing / Terminal.
- **next**: the next microstate (fixed) **or** a `next_fn(...)` that selects the next microstate when the state completes.
  - Required for loop-style Processing states like `Receiving_ReceivedToReady`.
- **segment**: `(from_macro, to_macro)` if this microstate is a dash between two macro blocks.
- **dash_index / dash_count**: position within the segment (for drawing partially filled dashes).
- **min_duration_seconds** *(only for Processing states that are time-based; e.g., closeout delay)*.

This table is also the single source of truth for:
- which states should flash (`kind == Input`)
- which tags to apply
- how the timeline should render

---

## ECS integration: tags for renderers

### Required behavior
- Every order stores **one authoritative microstate value** (e.g., `OrderState` in a component).
- A dedicated system mirrors that value into tags:
  - **Exactly one microstate tag** is enabled at a time (e.g., `Tag::OrderState_Requesting_NeedsInput`).
  - **Exactly one macro tag** is enabled at a time (e.g., `Tag::OrderMacro_RequestingItems`).

### Why both tags
- Micro tags: precise filtering (“show conveyor animation for `Receiving_*`”).
- Macro tags: simple grouping (“show orders that are in Boxing at all”).

---

## State update API (what to implement)

## Implementation details: answering the 4 key questions
This section is the “how” behind the plan. It answers:
1) what state we are in
2) when user input is marked complete
3) where the order is actually complete
4) how the next state is chosen

### 1) What state we are in
- **Authoritative source**: the order entity stores exactly one `OrderState` value (microstate).
  - Recommendation: create a dedicated component (e.g., `OrderWorkflow`) rather than stuffing more into the existing `Order` component.
- **Renderer/source-of-truth for queries**: a tag sync system mirrors the `OrderState` into:
  - **exactly one microstate tag** (e.g., `Tag::OrderState_Requesting_NeedsInput`)
  - **exactly one macro tag** (e.g., `Tag::OrderMacro_RequestingItems`)

### 2) When user input is marked “complete”
- **Only input systems advance Input states.** The tick system must never advance Input states.
- Definition: if `STATE_TABLE[state].kind == OrderStateKind::Input`, then the order stays in that state until the expected player action happens.
- When the player action is accepted by the input system:
  - set `state = next_state` (from the state table)
  - set `time_in_state = 0`
  - (optional) perform the state’s “on-exit/on-enter” actions

### 3) Where the order is actually complete
- **Terminal completion** is represented by reaching the microstate:
  - `OrderState::Complete_ClosedOut`
- The requirement “after READY TO SHIP, then 1 second later complete” is modeled as:
  - `Shipped_Stamp3` (**Input**) → `Complete_CloseoutDelay` (**Processing**, 1.0s) → `Complete_ClosedOut` (**Terminal**)
- Any cleanup (freeing a slot / removing from active list) should happen **when entering** `Complete_ClosedOut` (or in a cleanup system that queries the `Complete_ClosedOut` tag).

### 4) Where “next state after” comes from
The **only** source of truth for transitions is the `STATE_TABLE`. Do not rely on enum `+1`.

#### Required state table shape
```cpp
struct StateSpec {
  OrderMacroState macro;
  OrderStateKind kind;

  // For simple states:
  OrderState next;

  // For Processing states:
  float min_duration_seconds; // 0 if not time-based
  bool (*is_complete)(const Order& order,
                      float time_in_state
                      /* + world access as needed */);

  // Optional: for Processing states where next depends on conditions
  // (e.g., receiving loop deciding whether to go to AllReceived).
  // OrderState (*next_fn)(const Order& order /* + world access */);
};
extern const StateSpec STATE_TABLE[];
```

#### Two distinct advance paths (must both be implemented)
1) **Processing path (runs every frame for all orders)**:
   - `advance_if_complete(state, dt, time_in_state, order, world) -> state`
   - Behavior:
     - if `kind != Processing`: return `state`
     - else accumulate `time_in_state`, check `is_complete`, return `next` (or `next_fn`)

2) **Input path (runs on input events)**:
   - `try_advance_on_input(order, input_event) -> bool advanced`
   - Behavior:
     - if `kind != Input`: ignore
     - if the event matches this state’s expected input: set `state = next`, reset `time_in_state`

This split is what makes these simultaneously true:
- “Most states start with user input”
- “Then they take time to process”
- “Orders continue processing in the background”

### 1) Tick-based auto-advance (Processing only)
Implement a helper with this behavior:

- **Input/Terminal**: return current state.
- **Processing**: if the state is complete, return the next state; otherwise return current.

Recommended signature:

```cpp
OrderState advance_if_complete(OrderState current,
                               float dt,
                               float &time_in_state,
                               const Order &order /* or pointer */,
                               /* world queries as needed */);
```

Notes:
- `time_in_state` is required for time-based Processing states (like the 1s closeout delay).
- Processing completion can also be condition-based (e.g., “conveyor item reached ready area”).

### 2) Input-driven transitions
Separately from ticking, input handlers move an order forward when the player acts.
- Example: in `Requesting_NeedsInput`, a correct typed key transitions to `Requesting_AllRequested` when the last item is requested.
- Example: in `Shipped_Stamp2`, correct key transitions to `Shipped_Stamp3`.

**Important**: input transitions should never skip over Processing states; they should land on the next microstate listed in the table.

---

## Items: make “what’s left” obvious (explicit requirement)
Current code tracks items in multiple places (`items`, `selected_items`, `ready_items`) and sometimes mutates `items`, which makes it ambiguous.

**Implementation requirement** (pick this representation and stick to it):
- Add/maintain an immutable `required_counts` (e.g., `std::map<ItemType,int>`) that never changes after order creation.
- Maintain progress counts as additional maps:
  - `requested_counts`
  - `received_counts`
  - `boxed_counts`

UI and logic must use these counts so it is always trivial to compute:
- missing = required - received
- ready_to_box = received == required
- complete_items = boxed == required

If you want to keep lists for convenience, they must be derived from these counts (not the other way around).

---

## Implementation steps (intern checklist)

### Step 0 — Document the state table
- Create a single `STATE_TABLE` that contains metadata for every microstate:
  - `kind` (Input/Processing/Terminal)
  - `macro` (Incoming/Opened/…)
  - `next` (the next microstate)
  - `min_duration_seconds` (0 for non-timed processing states)
  - `is_complete(order, world, time_in_state)` predicate (for Processing states; for timed states this just checks duration)
  - timeline metadata: `segment_from_macro`, `segment_to_macro`, `dash_index`, `dash_count`
- Add a comment that this table is the single source of truth.

### Step 1 — Add the new order-state component
- Store the authoritative microstate.
- Add `time_in_state` (or equivalent) for Processing states.

### Step 2 — Tag sync system
- Implement a system that:
  - reads the order’s microstate
  - disables any old order-state tags
  - enables the correct micro + macro tag

### Step 3 — Tick (auto-advance) system
- Runs for **all orders** every frame.
- Uses `advance_if_complete` to advance **Processing** states.
- Guarantees forward-only progression.

Make completion rules explicit for the initial set:
- `Requesting_InputError`: timed (~0.25s–0.5s) feedback, then advance.
- `Requesting_AllRequested`: timed (0s) or immediate handoff.
- `Receiving_*`: condition-based (conveyor entities reaching threshold / counts updated).
- `Receiving_AllReceived`: timed (0s) handoff.
- `Complete_CloseoutDelay`: timed (1.0s).

### Step 4 — Input transition wiring
- Update the existing input systems to:
  - only advance when the current state is an Input state that matches the action
  - advance to the **exact next microstate** from the state table

### Step 5 — Timeline renderer update
- Render macro milestones as `[ MAJOR ]` blocks.
- Render Processing microstates as `-` dashes between major blocks using `dash_index/dash_count`.

### Step 6 — Debug tooling
- Add a tiny debug overlay/log line for each order:
  - `order_id`, `macro`, `microstate`, `kind`, `time_in_state`
- Add a way to toggle it on/off.

---

## Validation plan (how to verify each step)
This should be done incrementally as the intern implements.

### Validation A — State invariants
- **Exactly one microstate tag** per order at all times.
- **Exactly one macro tag** per order at all times.
- **Forward-only**: state never moves backward.

How to validate:
- Add an assertion/check in the tag sync system during development builds.

### Validation B — Background progression
- Start processing on an order (e.g., receiving/conveyor), switch to another order, confirm the first order continues to advance.

How to validate:
- Use the debug overlay: verify `time_in_state` advances and Processing microstates transition without selection.

### Validation C — Input vs Processing behavior
- In an **Input** state: verify ticking does not advance.
- In a **Processing** state: verify ticking advances when conditions are met.

How to validate:
- Force an order into each microstate (dev-only hotkeys or test setup) and observe.

### Validation D — Closeout behavior
- After `Shipped_Stamp3`, verify the order auto-completes after ~1 second:
  - `Shipped_Stamp3` → `Complete_CloseoutDelay` → (1s) → `Complete_ClosedOut`.

### Validation E — Timeline rendering
- Confirm the timeline:
  - shows correct macro milestone
  - fills the correct dash position while in Processing microstates
  - flashes only for Input microstates (when selected)

### Validation F — Items clarity
- For any selected order, verify the UI can always render:
  - required counts
  - requested counts
  - received counts
  - boxed counts
- Verify `required_counts` never changes after order creation.

---

## Notes on the current code (why this change is needed)
The current implementation mixes workflow, UI, and progress in `TimelineStageState`, and advances by numeric `+1`, which is fragile and hard to extend safely. The plan above replaces that with an explicit state table so adding/renaming/reordering states is safe and obvious.
