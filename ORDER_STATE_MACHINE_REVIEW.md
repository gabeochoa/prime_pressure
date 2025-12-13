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

A key simplification principle: **every “story beat” should map to exactly one primary state**, and anything else (UI flashing, stamp progress, item-by-item progress) should be *data attached to that state*, not additional “states.”

---

## Addendum: track microstates explicitly (nested under the story)
You asked to include microstates so we can track them. The clean way to do this is a **two-level model**:

- **Macro-state (story)**: one of a small set that matches the narrative.
- **Micro-state (interaction/transport/UI)**: the fine-grained step(s) within that macro-state.

This keeps the lifecycle story-readable *and* makes it possible to track/telemetry every small beat without exploding the primary state list.

### Proposed macro-states (story)
1) **Incoming**
2) **Opened**
3) **RequestingItems**
4) **ReceivingItems**
5) **ReadyToBox**
6) **Boxing**
7) **Shipped**
8) **Complete**

### Proposed microstates (what actually happens moment-to-moment)
Below is a concrete microstate list that matches current gameplay code + the narrative in `idea.md`.

#### Incoming
- **JustArrived**: spawned / visible in the list, not yet accepted
- **Backlogged** *(optional/future)*: present but not actionable due to capacity, quotas, etc.

#### Opened
- **SelectedActive**: player has this order “open” (active/selected)
- **SelectedInactive**: order exists but player is working another order

#### RequestingItems (warehouse request phase)
(In current gameplay, “request” is effectively the act of typing/matching items; in the story it’s a request to the warehouse.)
- **AwaitingFirstRequest**: opened but no items requested yet
- **AwaitingMoreRequests**: some items requested, not all requested yet
- **InputNeeded** *(UI microstate)*: order is selected and expects player input now
- **InputInvalid** *(UI microstate)*: player typed something that doesn’t advance the order (error feedback)

#### ReceivingItems (items in transit / arriving)
This is where today’s implementation has implicit transport microstates (conveyor + ready threshold).
- **QueuedNotOnConveyorYet** *(optional)*: requested but not spawned/moving
- **OnConveyorWaiting**: spawned on conveyor but not moving (not “released”)
- **OnConveyorMoving**: moving toward ready threshold
- **ReceivedToReadyArea**: crossed the threshold and counted as received/ready
- **PartialReceived**: some received, some still in transit (derived from counts)

#### ReadyToBox
- **AwaitingBoxingStart**: everything received, nothing packed yet
- **BoxingQueued** *(optional)*: multiple orders ready; this one is waiting its turn

#### Boxing
Boxing is already modeled as its own micro-FSM in the code (`BoxingState`), so list those explicitly:
- **FoldBox** *(present in enum, may or may not be used in current flow)*
- **PutItems**: place items into box (repeat until all placed)
- **Fold**: close box
- **Tape**: tape box
- **ShipActionReady**: waiting on the “ship” action

#### Shipped
There is an additional confirmation/closeout loop today (stamp progress) and `idea.md` also describes “press again to complete.” Track them as microstates:
- **ShipmentTriggered**: shipping action performed (box left the station)
- **AwaitingShipmentConfirmation**: needs confirmation/acceptance step(s)
- **StampProgress0**
- **StampProgress1**
- **StampProgress2**
- **StampProgress3**: confirmation complete

#### Complete
- **ClosedOut**: order removed from the active list / slot freed

### Interrupt microstates (overlay, not part of forward progression)
The cutscene text mentions a popup: “Accept reroute?” which implies a *modal interruption*. These should be tracked as **overlays** so they don’t break the forward-only main path:
- **ModalReroutePrompt**: order/system requires a decision (Y/N) before continuing
- **ModalTutorialPrompt** *(optional)*: “Press Enter to continue” gating

### Terminal outcome microstates (future pressure systems)
`idea.md` Phase 3 introduces timers/timeouts/quota pressure. Those will likely add terminal outcomes that should be tracked without complicating the happy path:
- **Late**: exceeded allowed time but still finishable (non-terminal)
- **FailedTimeout**: timed out; cannot be completed (terminal)
- **Cancelled** *(optional)*: customer cancelled
- **Returned/Refunded** *(optional, commentary phase)*

> Practical rule: the macro-state stays forward-only; overlays can appear/disappear; outcomes are terminal once reached.

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
For the same story readability, **do not add states** like:
- “flashing”
- “stamp 1/2/3”
- “selected vs ready”

Those should be fields that *explain why* the state is what it is.

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
