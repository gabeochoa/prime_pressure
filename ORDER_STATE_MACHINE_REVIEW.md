## Order system state machine review (simplification ideas)

### Scope / constraints
- **No code changes** in this pass. This document describes *what to change* and *why*.
- Goal: **it should be obvious what state every order is in** and **what items belong to the order / what’s left**.
- Goal: **easy forward-only progression** (orders only move forward).

---

## Reframing: the state machine should match the story
You described the core story arc as:

1) customer places order
2) order comes in
3) you open an order
4) start requesting the items from the warehouse
5) items are received from the warehouse
6) items are ready to be boxed up
7) items are boxed up
8) items are shipped
9) order complete

A key simplification principle: **every “story beat” should map to exactly one macro-state**. If you want “next step is obvious” without hidden progress, model **progress steps** (like stamp 0/1/2/3) as explicit microstates, but keep **UI effects (like flashing)** as derived rendering behavior from `(state, selection)`.

---

## Addendum: track microstates explicitly (nested under the story) — **explicit next-step, easy to extend**
Goal alignment update:
- **Flashing does not need to be a state** (treat it as UI behavior derived from state + selection).
- Some progress steps (like **stamp 0/1/2/3**) *can* remain explicit states if you want “next step is obvious” with no hidden counters.
- It should be **easy to add new states** without breaking existing transitions.

### Design rule
- **One active order-state at a time** (an enum value).
- States are **ordered forward-only**.
- Every state has an obvious **next state** (prefer a small transition table; avoid relying on `+1` enum math).
- UI behaviors like flashing are **state metadata** (how to render), not a stored per-order flag and not a distinct lifecycle state.

### Key requirement: “doesn’t need user input but is not complete”
Support this by splitting states into three kinds:
 - **Input states**: progress can only occur via an explicit player action (typing/pressing a key/choosing a modal prompt).
   - **UI**: flash (when selected) because the order is waiting on the player.
   - **Tick behavior**: `advance_if_complete(state, dt)` returns the **same state**.
 - **Processing states**: no user input needed; the game is “working” (items moving, delays, animations).
   - **UI**: no flash; show “in progress” / “moving”.
   - **Tick behavior**: once complete, `advance_if_complete` returns the **next state**.
 - **Terminal states**: finished; no transitions.

This matches your observation: **many states start with needed user input, then take time to process before moving on**.

### Proposed helper: auto-advance only when complete
Your desired shape (“given current state and `dt`, return the new state if it’s complete”) fits naturally as:

`advance_if_complete(current_state, dt) -> state`

- For **Input** and **Terminal** states, it returns `current_state`.
- For **Processing** states, it returns `next_state` once the completion condition is met.

Conceptually:

```cpp
// Pseudocode: designed to be easy to read and extend.
OrderState advance_if_complete(const OrderState current,
                               float dt,
                               float &time_in_state /* per-order */) {
  const StateSpec &spec = STATE_TABLE[current];
  if (spec.kind == StateKind::Input || spec.kind == StateKind::Terminal) {
    return current;
  }

  // Processing state: accumulate time and auto-advance when done.
  time_in_state += dt;
  if (time_in_state >= spec.min_duration_seconds /* or other condition */) {
    time_in_state = 0.0f;
    return spec.next;
  }
  return current;
}
```

Notes:
- You still need an **input event handler** to move from an Input state to its next state at the moment the player acts.
- `time_in_state` is not a “flag”; it’s the per-order timer needed to make Processing states advance with `dt`.
- If a Processing state completes based on a non-time condition (e.g., “all conveyor items reached ready”), the `spec` can hold a predicate instead of a duration.
### Macro-states (story beats)
Incoming → Opened → RequestingItems → ReceivingItems → ReadyToBox → Boxing → Shipped → Complete

### Microstates (explicit, ordered, forward-only)
Each line shows **State → Next state**.

#### Incoming
- **Incoming_Arrived** *(Input)* → **Opened_Active**
- **Incoming_Backlogged** *(optional/future)* → **Incoming_Arrived**

#### Opened
- **Opened_Active** *(Input)* → **Requesting_NeedsInput**
- **Opened_Inactive** *(optional)* → **Opened_Active**

#### RequestingItems (typing/request loop + attention)
- **Requesting_NeedsInput** *(Input)* → **Requesting_InputError** *(on invalid input)* or → **Requesting_AllRequested**
- **Requesting_InputError** *(Processing)* → **Requesting_NeedsInput**
- **Requesting_AllRequested** *(Processing)* → **Receiving_OnConveyorWaiting**

**Flashing (UI behavior, not a state):**
- Policy: **flash whenever the current state needs user input**.
- Concretely: a state “needs user input” if progress can only happen via an explicit player action (typing a key, pressing a button, making a modal decision), i.e. the next transition is gated by input rather than time/transport.
- This is derived from `(state, is_selected)` and a simple table of “input-driven states” (or metadata on each state), and does not require storing “flash” as an order state.

#### ReceivingItems (transport microstates)
- **Receiving_OnConveyorWaiting** *(Processing)* → **Receiving_OnConveyorMoving**
- **Receiving_OnConveyorMoving** *(Processing)* → **Receiving_ReceivedToReady**
- **Receiving_ReceivedToReady** *(Processing)* → **Receiving_OnConveyorWaiting**
- **Receiving_AllReceived** *(Processing)* → **ReadyToBox_Waiting**

*(Important: “AllReceived” is a **state** you enter exactly once when the last required item is received.)*

#### ReadyToBox
- **ReadyToBox_Waiting** *(Input)* → **Boxing_FoldBox** *(or → Boxing_PutItems if you skip FoldBox)*
- **ReadyToBox_Queued** *(optional/future)* → **ReadyToBox_Waiting**

#### Boxing (explicit step-FSM)
- **Boxing_FoldBox** *(Input)* → **Boxing_PutItems**
- **Boxing_PutItems** *(Input)* → **Boxing_Fold**
- **Boxing_Fold** *(Input)* → **Boxing_Tape**
- **Boxing_Tape** *(Input)* → **Boxing_Ship**
- **Boxing_Ship** *(Input)* → **Shipped_Stamp0**

#### Shipped (explicit stamp/confirmation steps)
- **Shipped_Stamp0** *(Input)* → **Shipped_Stamp1**
- **Shipped_Stamp1** *(Input)* → **Shipped_Stamp2**
- **Shipped_Stamp2** *(Input)* → **Shipped_Stamp3**
- **Shipped_Stamp3** *(Input)* → **Complete_ClosedOut**

#### Complete
- **Complete_ClosedOut** *(Terminal)*

### Timeline rendering idea: major blocks with minor “dash” steps between
You suggested a timeline that looks like:

`[ MAJOR ] - - - [ MAJOR ] - - [ MAJOR ] ...`

where each `-` is a **minor processing microstate** between two **major story blocks**.

Here’s a clean rule set that matches the “Input vs Processing” model above and keeps “what’s next?” obvious:

#### Definitions
- **Major block**: a story milestone that typically **starts with user input** (“do the thing”) and then hands off to processing.
  - Example: “Request items”, “Start boxing”, “Ship”, “Stamp/Confirm”, “Close out”.
- **Minor dash**: a **processing microstate** that advances via `advance_if_complete(state, dt)`.
  - Example: “on conveyor waiting/moving/received”, “error flash cooldown”, “shipping animation”, etc.

#### Practical mapping (recommended)
- Treat **Input states** as **major blocks** (they are the “decision/action points”).
- Treat **Processing states** as **minor dash segments** (they are the “in-flight” progress between majors).
- Terminal is a major endpoint.

This yields a timeline where:
- **Major nodes** = “what the player must do next” (also where flashing applies when selected)
- **Dashes** = “the system is working; wait a moment”

#### How to make this easy to render (state metadata)
For every `OrderState`, define:
- **major_index**: which major node this belongs to on the story timeline (0..N-1)
- **minor_index** and **minor_count**: if it’s a Processing state, which dash segment (0..minor_count-1) between `major_index` and `major_index+1`
- **kind**: Input / Processing / Terminal (already described above)

Then timeline UI can do:
- draw all major nodes (labels/icons)
- for the *current major segment*, fill in `minor_index` of `minor_count` dashes

#### Example (illustrative, using existing microstates)
Between the major “Requesting” action and the major “ReadyToBox” action, the dashes could be:
- `Receiving_OnConveyorWaiting` (dash 1)
- `Receiving_OnConveyorMoving` (dash 2)
- `Receiving_ReceivedToReady` (dash 3)
- (repeat until `Receiving_AllReceived`, then land on the next major)
- `Receiving_OnConveyorWaiting` (dash 1)
- `Receiving_OnConveyorMoving` (dash 2)
- `Receiving_ReceivedToReady` (dash 3)
- (repeat until `Receiving_AllReceived`, then land on the next major)

If you later add more transport nuance, you add/remove dash states **inside that one segment** without changing the high-level story nodes.

> This structure also makes adding states easy: add a new Processing microstate as another dash in a segment, or add a new Input state as a new major node.

### Interrupt microstates (explicit overlays)
These are still states (no booleans), but they temporarily override input and then return to the underlying main state:
- **Overlay_ReroutePrompt** → (return to previous)
- **Overlay_TutorialPrompt** *(optional)* → (return to previous)

### Future pressure outcomes (explicit terminal states)
If/when timers/quota are added, keep them explicit (no flags) by branching to terminal outcome states:
- **Outcome_Late** (non-terminal; can be an explicit state if you want it to affect gameplay)
- **Outcome_FailedTimeout** (terminal)
- **Outcome_Cancelled** *(optional terminal)*
- **Outcome_ReturnedRefunded** *(optional terminal)*

> Practical rule: the main chain is forward-only; overlays are temporary; outcome states are terminal branches.

### Making it easy to add new states (recommended pattern)
To make extension safe and obvious, define states and transitions **declaratively** in one place:
- **Enum**: add the new state name.
- **Transition table**: add one row describing:
  - **next** (the intended forward step)
  - **guard** (when it is allowed)
  - **action** (what data updates happen on transition, if any)
  - **UI metadata** (e.g., “flash when selected”, “show ‘boxing needed’ banner”)

This avoids fragile “`+1` means next” coupling while keeping “what’s next?” readable in a single list.

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
- “Is this order waiting to be opened, or already being worked?”
- “Is this order shipped, or only partially stamped?”

### 2) Numeric `+1` transitions are fragile
Advancing by `+1` makes transitions **implicit** and **hard to audit**.
- Adding/reordering enum values can silently break runtime behavior.
- It’s difficult to see the intended graph (which edges exist and why).

### 3) UI state is embedded in business state
`ConveyorActiveFlash` is a UI effect (“selected => flash”), but it is stored as a distinct workflow state. That creates extra “states” that are not meaningful to the order’s actual lifecycle.

### 4) “Stamp progress” being modeled as states multiplies complexity
Representing `ReadyStamp0..3` as separate FSM states causes a state explosion and makes “shipped-ness” ambiguous (e.g., shipped vs. fully stamped vs. complete).

### 5) Item truth is split across multiple fields
The system tracks item progress in multiple vectors/counters, and at least one system treats `items` as mutable “remaining items.” Even if it works, it makes “what items are in this order?” less obvious.

---

## Recommendation: build a story-first order lifecycle

### The primary state should be story-readable
Use a single, small set of order states that read like the narrative.

A good story-first set (forward-only) is:

1) **Incoming** *(order comes in)*
2) **Opened** *(you open the order / commit to working it)*
3) **RequestingItems** *(you are requesting items from the warehouse)*
4) **ReceivingItems** *(items are arriving / moving through fulfillment)*
5) **ReadyToBox** *(all required items are received and staged)*
6) **Boxing** *(packing interaction in progress)*
7) **Shipped** *(shipment action complete; may require stamping/confirmation)*
8) **Complete** *(fully finished; slot can be freed)*

This maps 1:1 to your story beats; the only “compression” is that “customer places order” is *pre-game world state* and can be represented as the moment an order entity is created.

### Keep progress out of the state list
For story readability, keep the **macro-states** free of UI/progress detail (i.e., don’t turn the *story* state list into 50 states).

However, if the design goal is “**no flags** and the next step is always obvious”, then it’s reasonable to model things like **flashing** and **stamp 0/1/2/3** as **microstates** (see the addendum above) while keeping the macro story beats unchanged.

---

## Make item status obvious (recommended regardless of FSM approach)

### One canonical definition for “items in the order”
Pick a single canonical representation for the order’s contents and keep it immutable:
- **Preferred**: `required_counts: map<ItemType,int>`
- **Acceptable**: `required_items: vector<ItemType>` but never mutate it

Then represent progress with counts (these are naturally forward-only):
- `requested_counts` (what you asked the warehouse for)
- `received_counts` (what has arrived)
- `boxed_counts` (what is packed)

From those, you can derive the most important “obvious” views:
- **What items are in this order?** → `required_*`
- **What’s missing?** → `required - received`
- **What’s ready but not boxed?** → `received - boxed`
- **What’s done?** → `boxed == required`

This also makes UI easy: you can show a per-item row as `received/required` and `boxed/required`.

---

## Two implementation styles (conceptual)

## Option A (simplest): derived story state from progress (no explicit FSM)

### Concept
Store only minimal forward-only facts (counts + flags), and compute the story state as a pure function.

### Example derived story rules (illustrative)
- `Incoming` if created but not opened
- `Opened` if opened but no requests made
- `RequestingItems` if `requested < required`
- `ReceivingItems` if `requested == required` but `received < required`
- `ReadyToBox` if `received == required` and not boxing started
- `Boxing` if boxing started and `boxed < required`
- `Shipped` if boxing finished and shipping triggered
- `Complete` if shipment confirmed (e.g., stamp/ack complete)

### Why this fits the story well
- You can always explain the state with a number: “Received 2/3 items, waiting on 1.”
- It’s hard to get stuck in an invalid state.

## Option B: explicit forward-only FSM with story states

### Concept
Keep an explicit FSM, but keep it story-readable and keep the transition table small.

### Events (story actions) and transitions
- `OrderArrived`: (creates entity) → Incoming
- `OpenOrder`: Incoming → Opened
- `RequestNextItem` / `RequestAllItems`: Opened/RequestingItems → RequestingItems
- `AllItemsRequested`: RequestingItems → ReceivingItems
- `ItemReceived`: ReceivingItems → ReceivingItems (stays)
- `AllItemsReceived`: ReceivingItems → ReadyToBox
- `BeginBoxing`: ReadyToBox → Boxing
- `BoxingFinished`: Boxing → Shipped
- `ConfirmShipment` (or `StampAdvanced`): Shipped → Complete

### Keep micro-steps as progress fields
If shipping confirmation is a minigame (like the existing stamp sequence), model it as:
- state: `Shipped`
- field: `ship_confirm_progress` (0..N)

Not separate FSM states.

---

## UI “attention” should be derived, not stored as a state

Today, the system uses a special state (`ConveyorActiveFlash`) to indicate “needs attention.” In a story-first model:
- compute `attention_reason` from (selected/active order) + (derived state) + (missing counts)
- examples:
  - `attention_reason = NeedsWarehouseRequest` when Opened/RequestingItems
  - `attention_reason = NeedsBoxing` when ReadyToBox
  - `attention_reason = NeedsShipmentConfirm` when Shipped but confirm_progress < N

This keeps the order lifecycle clean and narrative-aligned.

---

## If using a header-only FSM library is helpful
If you choose Option B (explicit FSM), a header-only FSM can make the transition table obvious and enforce “forward-only” by construction.

### Good header-only candidates
- **Boost.SML** (`boost::sml`): very expressive for “events + guards + actions”.
- **tinyfsm**: small and approachable; less compile-time machinery.

If you choose Option A (derived state), a library is usually unnecessary.

---

## Concrete simplification steps (conceptual migration plan)
(These are steps you could take later; not performed here.)

1) **Rename/reframe states around the story** (Incoming → Opened → … → Complete).
2) **Separate UI effects** from lifecycle (flash/attention becomes a derived flag).
3) **Replace stamp-as-states** with `ship_confirm_progress`.
4) **Choose a single “items in order” truth** (immutable required items) and represent progress with counts.
5) Add a single `OrderSnapshot`/`order_status_string()` generator so every screen shows the same story state and the same item breakdown.

---

## Bottom line
The system becomes much less complicated if the lifecycle is defined in the same language as the game story, and everything else is modeled as **progress data**. That yields a small forward-only state set where you can always answer:
- **Where am I in the story?** (state)
- **What items are in this order and what’s left?** (required vs received/boxed)
- **What should the player do next?** (derived attention reason)
