## Order system state machine review (simplification ideas)

### Scope / constraints
- **No code changes** in this pass. This document describes *what to change* and *why*.
- Goal: **it should be obvious what state every order is in** and **what items belong to the order / what’s left**.
- Goal: **easy forward-only progression** (orders only move forward).

---

## What exists today (as observed)

### Current “workflow state” representation
The order workflow is represented by `Order::timeline.state` (type `TimelineStageState`). That enum currently mixes:
- **High-level stages**: Conveyor / Boxing / Ready / Ship
- **Phase**: Pending / Active / Done (derived by `timeline_phase()`)
- **UI-only concerns**: `ConveyorActiveFlash`
- **“Stamp progress”**: `ReadyStamp0..ReadyStamp3`

The update logic advances states by **numeric increment** (`get_next_state()` returns `static_cast<TimelineStageState>(int(current) + 1)`). This implicitly relies on the enum values being laid out in a very specific numeric order.

### Current “items in the order” representation
`Order` currently keeps multiple item vectors / counters:
- `items` (commonly treated as the set of items the order requires)
- `selected_items` (items “matched” during conveyor input)
- `ready_items` (items that reached the “ready” threshold)
- `items_completed` (an additional counter)

In most of the conveyor pipeline, `items` appears to be the **required list**, while `selected_items` and `ready_items` represent progress. However, `BoxItemSystem` mutates `order.items` by **erasing** items as they are boxed (treating `items` like “remaining items”), which makes the meaning of `items` inconsistent across the codebase.

---

## Why the current state machine feels complicated / non-obvious

### 1) One enum is doing several jobs
`TimelineStageState` simultaneously encodes:
- “Where am I in the process?” (stage)
- “Is this step active/pending/done?” (phase)
- “Should the UI flash?” (attention state)
- “How many stamp steps have been completed?” (progress)

That makes it hard to answer simple questions like:
- “Is this order currently waiting for conveyor input, or done with conveyor?”
- “Is this order shipped, or only partially stamped?”

### 2) Numeric `+1` transitions are fragile
Advancing by `+1` makes transitions **implicit** and **hard to audit**.
- Adding/reordering enum values can silently break runtime behavior.
- It’s difficult to see the intended graph (which edges exist and why).

### 3) UI state is embedded in business state
`ConveyorActiveFlash` is a UI effect (“selected => flash”), but it is stored as a distinct workflow state. That creates extra “states” that are not meaningful to the order’s actual lifecycle.

### 4) “Stamp progress” being modeled as states multiplies complexity
Representing `ReadyStamp0..3` as separate FSM states causes a state explosion and makes “shipped-ness” ambiguous (e.g., shipped vs. fully stamped vs. complete).

### 5) Some guards/interpretations appear inconsistent
From reading the code, there are a few patterns that make state harder to trust:
- A forward increment can land on values whose meaning is unclear or whose phase flags don’t match other helpers.
- Item progress is tracked in multiple places (vectors + counters), and at least one system mutates `order.items` as a “remaining” list.

Even if everything “works in practice,” these patterns make the system **hard to reason about**.

---

## Recommendation: separate the concerns
A simpler, more obvious design is to separate:
1) **Workflow stage** (what step of the pipeline the order is in)
2) **Progress within the stage** (counts, stamp progress)
3) **UI attention** (flash, “input needed”) derived from selection + stage
4) **Item state** represented from a single source of truth

Below are two good options.

---

## Option A (simplest): derive state from progress (no explicit FSM)

### Concept
Instead of storing a detailed state machine value, store only the minimal forward-only “facts” about progress, and compute the display/logic state as a pure function.

### Suggested forward-only facts
- **Required items**: immutable definition (e.g., `required_items` or `required_counts`)
- **Selection progress**: how many items have been matched/selected
- **Ready progress**: how many items have reached the ready queue
- **Boxing progress**: either “boxing complete” or “boxed count”
- **Shipping/stamp progress**: `stamp_progress` 0..3
- **Completion flag**: `is_complete` (or “slot cleared” is the completion)

### Derived state function (illustrative)
Compute a single “headline state” for the UI and logic:
- **Collecting**: not all required items are ready
- **ReadyToPack**: all required items are ready, not yet boxed
- **Packing**: boxing in progress (or if an active boxing session exists)
- **Stamping**: shipped but stamp progress < 3
- **Complete**: stamp progress == 3 (or slot cleared)

### Why this is nice
- **No invalid states**: the state is always consistent with the data.
- **Forward-only by construction**: counts/progress only increase.
- **Obviousness**: you can show the state by showing the underlying counts.

### Where UI flash fits
Instead of encoding a flash state:
- `needs_attention = (selected && state == Collecting && missing_items > 0)`
- or `attention_reason = ConveyorInputNeeded | BoxingNeeded | StampingNeeded | None`

This keeps UI effects out of the lifecycle.

---

## Option B: explicit forward-only FSM with a small state set

### Concept
Keep an explicit FSM, but keep the number of states small and make transitions explicit in a table.

### Proposed minimal workflow states
A practical small set (5 states):
1) **Collecting** (conveyor input / items moving)
2) **QueueReady** (all items ready; waiting to begin packing)
3) **Packing** (boxing in progress)
4) **Stamping** (shipping done; stamp progress 0..3)
5) **Complete** (order finished; slot can be freed)

### Events and guards (forward-only)
Model transitions as events with guards:
- `OrderCreated` -> Collecting
- `AllItemsReady` -> QueueReady
- `BeginPacking` -> Packing
- `PackingFinished` -> Stamping (stamp_progress=0)
- `StampAdvanced` (progress++) stays in Stamping
- `StampComplete` -> Complete

Guards make it obvious why a transition is allowed:
- `AllItemsReady` guard: `ready_count == required_count`
- `BeginPacking` guard: state==QueueReady
- `PackingFinished` guard: boxing session done
- `StampAdvanced` guard: next expected key

### Why this is nice
- **Explicit graph**: easy to read and reason about.
- **Easy to move forward**: transitions are just event applications.
- **Still obvious**: state name is meaningful; progress fields are separate.

---

## Make item progress obvious (recommended regardless of Option A/B)

### Single source of truth for “what items are in the order”
Pick one canonical representation for required items and keep it immutable:
- **Best for clarity**: `std::map<ItemType,int> required_counts`
- **Also OK**: `std::vector<ItemType> required_items` but never mutate it

Then represent progress as counts (not additional lists you have to reconcile):
- `selected_counts` (matched / keyed)
- `ready_counts` (arrived at ready)
- `boxed_counts` (packed)

From these, you can derive:
- **Missing** = required - ready
- **Still needs input** = required - selected - on_conveyor
- **Ready but unboxed** = ready - boxed

### Why counts are clearer than multiple vectors
- Vectors allow duplicates and need reconciliation logic.
- Counts make the UI and guard checks straightforward and cheap.

### Resulting UI clarity
For each order card, you can show:
- **State**: `Collecting / ReadyToPack / Packing / Stamping / Complete`
- **Items**: required counts + per-item progress bars (selected/ready/boxed)
- **Next action**: derived “attention reason”

---

## If using a library is helpful (header-only suggestions)

If you prefer explicit FSMs with a transition table, a header-only library can enforce correctness.

### Good header-only candidates
- **Boost.SML** (`boost::sml`): compile-time FSM, very explicit transitions, good for “events + guards + actions”.
- **tinyfsm**: lightweight, simple, more runtime-oriented.

### What a library would buy you
- A single place to define the state graph.
- Compile-time checking (depending on library) that transitions are valid.
- Cleaner “apply_event(order, event)” flow.

### When not to use a library
If you adopt **Option A (derived state)**, a library likely adds more complexity than value.

---

## Concrete simplification steps (conceptual migration plan)
(These are steps you could take later; not performed here.)

1) **Define canonical required items** (immutable) and stop using `items` as a mutable “remaining” list.
2) Replace `TimelineStageState` with either:
   - Option A: a derived state function + minimal progress fields, or
   - Option B: a small explicit FSM state + separate progress fields.
3) Move `ConveyorActiveFlash` out of the lifecycle and into a derived UI flag.
4) Replace `ReadyStamp0..3` as states with a single `Stamping` state + `stamp_progress`.
5) Add a single `order_status_string()`/`OrderSnapshot` concept for UI so “what state is this in?” is always consistent.

---

## Bottom line
The current system is complicated mainly because **workflow, UI effects, and progress are fused into one enum**. Splitting those concerns and using either **(A) derived state** or **(B) a small explicit FSM** will make it much more obvious what each order is doing and what items remain, while keeping the “forward-only” rule easy to enforce.
