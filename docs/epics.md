# Prime Pressure - Development Epics

## Epic Overview

| ID | Title | Priority | Description |
|----|-------|----------|-------------|
| **E01** | **Core Fulfillment Loop** | P0 (MVP) | The baseline "Order -> Box -> Ship" gameplay. (Already mostly implemented). |
| **E02** | **Oppression Systems** | P0 (MVP) | The mechanics that apply pressure: TOT Timer, Smile Checks, Daily Pledges. **MVP requires at least one pressure hook** (TOT recommended) inside Days 1–3. |
| **E03** | **Economy & Meta** | P0→P1 | Day/Run progression logic (Day Loop). **MVP target:** a **Day 1–3 vertical slice** endpoint at End of Day 3. Economy comes later. |
| **E04** | **Content & Narrative** | P1 | Data-driven content: Item Tiers (1-3), Email System, and Glitch Progression. **MVP includes a minimal end-of-day email/review delivery for Days 1–3** (not the full narrative system). |
| **E05** | **Game Juice & Polish** | P2 | Visual/Audio polish: CRT Shaders, Camerashake, Dynamic Audio Manager. |

---

## Epic Breakdowns

### MVP Definition (Option A) — Day 1–3 Vertical Slice

The MVP deliverable is a cohesive **Days 1–3** slice that proves the **Day Loop** works:

- **Start-of-day:** pledge ritual
- **Shift:** fulfillment gameplay
- **End-of-day:** review/email delivery + day summary
- **Hard endpoint:** “Day 3 Complete”

This MVP must preserve the Prime Pressure input rules (Shift encoded in data with `^`; no direct Shift checks in gameplay code) and avoid softlocks across day transitions.

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
    *   [ ] **MVP constraint:** TOT only applies during **Shift** (not during pledge/review screens).

**Story 2.2: Smile Verification System**
*   **As a** System,
*   **I want** to interrupt the player with a "Smile Check",
*   **So that** I can ensure compliance and break their flow.
*   **Acceptance Criteria:**
    *   [ ] Randomly triggers during the `Work Phase`.
    *   [ ] Pop-up covers center screen: "Please Smile for Verification".
    *   [ ] Player must hold a **non-Shift** key for 1 second to clear (Shift checks are prohibited in gameplay code; Shift is encoded via `^` in data instead).
    *   [ ] Failure to smile pauses all inputs and raises TOT.
    *   [ ] **MVP note:** Smile Verification is optional for the Day 1–3 slice if TOT is the chosen minimal pressure hook.

**Story 2.3: Morning Pledge Minigame**
*   **As a** Company,
*   **I want** the player to type a loyalty pledge before starting the day,
*   **So that** they are indoctrinated.
*   **Acceptance Criteria:**
    *   [ ] New Day Phase: `Pledge` (e.g., stored on the Sophie singleton entity as `CampaignProgress.phase`).
    *   [ ] Text crawler shows the daily slogan.
    *   [ ] Player must type it 100% correctly to begin the Shift phase.
    *   [ ] **MVP scope:** implement pledges for Days 1–3 (content can be simple and escalate lightly).

---

### Epic 3: Economy & Meta (P1)

**Goal:** Implement the Day Loop state machine and (later) the resource loop (Earn Tokens -> Buy Survival).

#### Stories

**Story 3.0: Day Loop State Machine (Days 1–3 Vertical Slice)**
*   **As a** Player,
*   **I want** the game to advance through Day 1–3 as a clear ritualized loop (Pledge → Shift → Review),
*   **So that** the experience feels like a campaign, not a disconnected sandbox.
*   **Acceptance Criteria:**
    *   [ ] The Sophie singleton entity stores `day_index` (1..3) and `phase` (Pledge/Shift/Review).
    *   [ ] Completing pledge moves to Shift; completing Shift moves to Review; completing Review advances the day.
    *   [ ] End-of-Day 3 reaches a hard “Day 3 Complete” endpoint.
    *   [ ] No known softlocks across Day 1–3 transitions.
    *   [ ] **MVP telemetry hooks:** `day_start`, `day_end`, `run_completed(day=3)` or `run_ended(reason=...)`.

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

**Story 3.4: Corporate Directory Screen**
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
    *   [ ] Support for input modifiers **without direct Shift checks in gameplay code**.
    *   [ ] Shifted requirements are expressed via the `^` encoding in data/config and resolved by the input layer.
    *   [ ] Visual indicator on the typing prompt showing the required modifier.

**Story 4.3: Email Narrative System**
*   **As a** Storyteller,
*   **I want** to push emails to the player's inbox between shifts,
*   **So that** I can deliver lore and foreshadowing.
*   **Acceptance Criteria:**
    *   [ ] **MVP subset:** end-of-day Review phase shows at least one “email/message” per day for Days 1–3.
    *   [ ] Full system later: `EmailManager` queues messages based on Day #.
    *   [ ] Full system later: unread badge on "Email" icon.
    *   [ ] Full system later: "Forwarded" chains that show corporate conversations.

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

