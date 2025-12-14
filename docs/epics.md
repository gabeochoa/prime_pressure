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
- **Work phase:** fulfillment gameplay
- **End-of-day:** review/email delivery + day summary
- **Hard endpoint:** “Day 3 Complete”

This MVP must preserve the Prime Pressure input rules (keyboard Shift encoded in data with `^`; no direct keyboard Shift checks in gameplay code) and avoid softlocks across day transitions.

### PRD FR Coverage Map (MVP: Day 1–3 Slice)

This is an explicit traceability map for the PRD’s “Success Criteria + MVP Scope” requirements.

| FR | Requirement (short) | Epic / Story coverage |
|---|---|---|
| FR1 | Clarify/stabilize core fulfillment loop | E01 1.1, E01 1.2 |
| FR2 | Day Loop Days 1–3 (Pledge → Work → Review) with Day 3 endpoint | E03 3.0 (primary), E02 2.3, E04 4.3 (MVP subset) |
| FR3 | Morning pledge ritual (Days 1–3) | E02 2.3 |
| FR4 | End-of-day review/email + day summary + “Day 3 Complete” endpoint | E03 3.0 (endpoint + review flow), E04 4.3 (MVP subset content) |
| FR5 | At least one pressure hook (TOT recommended) in Days 1–3 | E02 2.1 (TOT), E02 2.2 (optional Smile) |
| FR6 | Minimum event logging (session/day/run) | E03 3.0 |
| FR7 | Metric semantics (start/complete definitions + quit-day + exit reasons) | E03 3.0 |

### Epic 1: Core Fulfillment Loop (P0 - MVP)

**Goal:** Solidify the "happy path" of receiving, typing, boxing, and shipping an order.
**Status:** Mostly Implemented (Refining existing ECS).

#### Stories

**Story 1.1: Refine Boxing Input State**
*   **As a** Player,
*   **I want** the boxing phase to accept typed input clearly (Place -> Fold -> Tape -> Label),
*   **So that** I can complete the physical shipping process.
*   **Acceptance Criteria:**
    *   **Given** I am in the Boxing phase of an active order during the Work phase  
        **When** I enter the required boxing key sequence for the current step (Place → Fold → Tape → Label)  
        **Then** the boxing step advances and the UI reflects the new step.  
    *   **Given** I am at the Tape step  
        **When** I perform the Tape sequence correctly  
        **Then** the Tape step completes and the workflow advances to the next step.  
    *   **Given** I complete the final boxing step for the order  
        **When** the order is ready to ship  
        **Then** the order enters an `OrderShipped`/shipped-complete state and is no longer treated as in-progress.

**Story 1.2: Order Monitor UI**
*   **As a** Player,
*   **I want** to see the incoming stream of orders on the "Computer Screen",
*   **So that** I know what items are coming next.
*   **Acceptance Criteria:**
    *   **Given** I am in the Computer view during the Work phase  
        **When** I look at the order monitor  
        **Then** I see the current active order and the next 3 queued orders.  
    *   **Given** the MVP is “Locked/Linear” order flow  
        **When** new orders arrive  
        **Then** I cannot reject them (only progress through the queue).  
    *   **Given** I am in the Computer view during the Work phase  
        **When** I press `[TAB]`  
        **Then** the active view switches between Computer and Warehouse.

---

### Epic 2: Oppression Systems (P0 - MVP)

**Goal:** Implement the "Pressure" mechanics that force the player to panic.

#### Stories

**Story 2.1: Time Off Task (TOT) Timer**
*   **As a** Manager (System),
*   **I want** to track player inactivity,
*   **So that** I can punish them for being slow.
*   **Acceptance Criteria:**
    *   **Given** `CampaignProgress.phase == Work`  
        **When** no gameplay-relevant input is detected for > 2 seconds  
        **Then** the TOT meter begins filling.  
    *   **Given** the TOT meter reaches its warning threshold  
        **When** the threshold is crossed  
        **Then** a warning SFX plays and a clear warning overlay is shown.  
    *   **Given** I have received 3 warnings  
        **When** the third warning is applied  
        **Then** a Strike/Fine is recorded (exact penalty can be placeholder for MVP).  
    *   **Given** `CampaignProgress.phase == Pledge` or `CampaignProgress.phase == Review`  
        **When** time passes without input  
        **Then** TOT does not advance (Work-only constraint).

**Story 2.2: Smile Verification System**
*   **As a** System,
*   **I want** to interrupt the player with a "Smile Check",
*   **So that** I can ensure compliance and break their flow.
*   **Acceptance Criteria:**
    *   **Given** `CampaignProgress.phase == Work`  
        **When** a Smile Check triggers  
        **Then** a blocking pop-up is shown: “Please Smile for Verification”.  
    *   **Given** the Smile Check pop-up is active  
        **When** I hold the configured **non-keyboard-Shift** key for 1 second  
        **Then** the pop-up clears and the Work phase resumes.  
    *   **Given** the Smile Check pop-up is active  
        **When** I do not complete the hold requirement within the allowed window (MVP: choose a single fixed window, or “until cleared”)  
        **Then** the game applies a penalty (MVP: increment TOT/strike pressure) and blocks Work gameplay inputs until the check is cleared.  
    *   **Given** `CampaignProgress.phase == Pledge` or `CampaignProgress.phase == Review`  
        **When** the game is in those phases  
        **Then** Smile Checks do not trigger (Work-only constraint).  
    *   **MVP note:** Smile Verification is optional for the Day 1–3 slice if TOT (Story 2.1) ships as the single pressure hook.

**Story 2.3: Morning Pledge Minigame**
*   **As a** Company,
*   **I want** the player to type a loyalty pledge before starting the day,
*   **So that** they are indoctrinated.
*   **Acceptance Criteria:**
    *   **Given** a new day begins (Day 1–3)  
        **When** the day starts  
        **Then** `CampaignProgress.phase` is set to `Pledge` and the pledge prompt is shown.  
    *   **Given** the pledge prompt is visible  
        **When** I type the pledge text exactly correctly  
        **Then** the day advances to `CampaignProgress.phase == Work`.  
    *   **Given** the pledge prompt is visible  
        **When** I type an incorrect character  
        **Then** the pledge does not complete and the UI indicates the error clearly (MVP: simplest possible feedback is acceptable).  
    *   **Given** the MVP slice is Days 1–3  
        **When** Day 1, Day 2, or Day 3 begins  
        **Then** a pledge exists for that day (text can be minimal; escalation is optional).

---

### Epic 3: Day Loop (P0) + Economy & Meta (P1)

**Goal:** Implement the Day Loop state machine and (later) the resource loop (Earn Tokens -> Buy Survival).

#### Stories

**Story 3.0: Day Loop State Machine (Days 1–3 Vertical Slice)**
*   **As a** Player,
*   **I want** the game to advance through Day 1–3 as a clear ritualized loop (Pledge → Work → Review),
*   **So that** the experience feels like a campaign, not a disconnected sandbox.
*   **Acceptance Criteria:**
    *   **Given** a new run begins  
        **When** the session starts and the player provides the first actionable input of Day 1  
        **Then** `session_start` is recorded and the run is considered “started” for completion-rate accounting.  
    *   **Given** the Sophie singleton entity exists  
        **When** the Day Loop is active  
        **Then** Sophie stores `day_index` (1..3) and `phase` (Pledge/Work/Review).  
    *   **Given** I complete the pledge for the current day  
        **When** pledge completion is detected  
        **Then** the game transitions to `phase == Work` and records `day_start(day_index)`.  
    *   **Given** I complete the shift for the current day  
        **When** shift completion is detected  
        **Then** the game transitions to `phase == Review` and records `day_end(day_index)`.  
    *   **Given** I complete the Review phase for Day 1 or Day 2  
        **When** I acknowledge the end-of-day summary/email screen  
        **Then** the game advances to the next day and returns to `phase == Pledge`.  
    *   **Given** I complete the Review phase for Day 3  
        **When** I acknowledge the end-of-day summary/email screen  
        **Then** the game reaches the hard endpoint “Day 3 Complete” and records `run_completed(day=3)`.  
    *   **Given** the run ends before completion  
        **When** I quit, fail, crash, or hit a softlock watchdog  
        **Then** `run_ended(reason=quit|fail|crash|softlock, day_index=<current>)` is recorded and `session_end` is recorded.  
    *   **Given** Day Loop transitions occur  
        **When** moving between Pledge → Work → Review → next day  
        **Then** there are no known softlocks across Days 1–3 (manual playtest checklist is acceptable for MVP).

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
    *   [ ] Support for input modifiers **without direct keyboard Shift checks in gameplay code**.
    *   [ ] Shifted requirements are expressed via the `^` encoding in data/config and resolved by the input layer.
    *   [ ] Visual indicator on the typing prompt showing the required modifier.

**Story 4.3: Email Narrative System**
*   **As a** Storyteller,
*   **I want** to push emails to the player's inbox between shifts,
*   **So that** I can deliver lore and foreshadowing.
*   **Acceptance Criteria:**
    *   **Given** `CampaignProgress.phase == Review`  
        **When** the end-of-day review screen is shown for Day 1–3  
        **Then** at least one “email/message” is shown and the player can acknowledge/dismiss it to continue.  
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

