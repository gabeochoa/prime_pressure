# Prime Pressure - Game Design Document

**Author:** Gabe
**Game Type:** Simulation
**Target Platform(s):** PC (Steam, Itch.io)

---

## Executive Summary

### Core Concept

A satirical warehouse fulfillment game where you type to fill orders while navigating an increasingly dystopian corporate environment that shifts from "business as usual" to a "war economy."

### Target Audience

Fans of "job simulator" games with deep narrative layers (*Papers, Please*, *Not Tonight*) and high-skill typing/management games (*Cook, Serve, Delicious!*).

### Unique Selling Points (USPs)

1.  **Mechanics as Narrative:** The typing gets harder *because* the world gets worse (Modifiers), not just arbitrary difficulty.
2.  **The "Glitch" Aesthetic:** The UI and Music degrade over time, mirroring the protagonist's mental state.
3.  **Satire of "Now":** Directly addresses the current cultural conversation around fulfillment centers and labor rights.

---

## Goals and Context

### Project Goals

To create a visceral, "stress-fun" experience that uses typing mechanics to critique the human cost of modern convenience, blending the frantic gameplay of *Cook, Serve, Delicious!* with the narrative oppression of *Papers, Please*.

### Background and Rationale

The "workplace dystopia" genre has a dedicated following. Players enjoy the tension between mastering complex mechanics and navigating a hostile narrative setting. *Prime Pressure* capitalizes on this by gamifying the familiar (and often criticized) experience of warehouse fulfillment.

---

## Core Gameplay

### Game Pillars

1.  **Narrative Action:** Mechanical inputs (typing) are directly tied to narrative tension. Complex inputs represent Dangerous items.
2.  **Satire via Systems:** The horror of the setting is conveyed through "Corporate Benevolence" mechanics (Smile Checks) that interrupt flow.
3.  **The Boiling Frog:** The game starts normal and slowly descends into dystopia without the UI ever acknowledging the horror.
4.  **Rhythmic Flow:** Mastery of the typing rhythm provides the "fun" counterweight to the narrative "stress."

### Core Gameplay Loop

1.  **Select Order:** Identify priority orders on the computer screen.
2.  **Fulfill:** Type item names to box them.
3.  **Comply:** Respond to "Smile Checks" and "Morning Pledges" to avoid penalties.
4.  **Ship:** Complete orders before the TOT (Time Off Task) timer runs out.

### Win/Loss Conditions

### Win/Loss Conditions
*   **Unique Endings:**
    *   **Loyalist Ending (The Good Drone):** Survive Day 20 with High Stars / Low Resistance.
        *   *Result:* Promotion to Middle Management. You are now the one sending the oppressive emails. (Bleak/Cyclical).
    *   **Resistance Ending (The Spark):** Survive Day 20 with High Resistance Points.
        *   *Result:* The warehouse goes on strike. The system halts. You are arrested, but you see the lines stop moving. (Heroic/Sacrificial).
    *   **Failure:** Fired (0 Stars) or "Disappeared" (Max Demerits).

---

## Game Mechanics

### Primary Mechanics

*   **Complex Typing:** Type item names to box them. Modifiers (e.g., `[SHIFT] + [!]`) are added for hazardous/narrative items.
*   **Time Off Task (TOT):** A passive timer that punishes inactivity.
*   **Smile Verification:** A "Wellness Check" popup requiring specific inputs to "smile".
*   **Morning Pledge:** Daily slogan typing ritual.

### Controls and Input

{{controls}}

---

## Simulation Specific Elements

### Core Simulation Systems

**Workflow Simulation:** The game simulates a specific workstation workflow in high fidelity.
1.  **Computer (Order View):** Player accepts orders.
2.  **Warehouse (Request View):** Player types codes to request items from deep storage to the conveyor.
3.  **Boxing (Assembly View):**
    *   **Place:** Move items from conveyor to box.
    *   **Fold & Tape:** Execute typing sequences/gestures to seal the box.
    *   **Label:** Apply shipping data.
    *   **Ship:** Send it down the line.

**What's being simulated:**

- Primary simulation focus (city, farm, business, ecosystem, etc.)
- Simulation depth (abstract vs. realistic)
- System interconnections
- Emergent behaviors
- Simulation tickrate and performance

### Management Mechanics

*   **Resource Management:**
    *   **Emergent/Environmental:** The story is told primarily through the *Item Database* and *Email Notifications*.
    *   **Emails:** Context for rules (e.g., "Jon was caught discussing <party> -> New Pledge: 'No Politics'").
    *   **Corporate Directory:** An evolving org chart that tracks the staff's descent from corporate drones to militarized fanatics (or "missing" persons).
    *   **Time:** The primary constraint (TOT / Quota).
    *   **Inventory Space:** Limited conveyor/box space requires "Tetris-lite" management.
*   **Strategic Decisions:**
    *   **Order Selection (Conditional):** normally **Linear/Forced**.
    *   **Off-Books Orders (Resistance):** Optional "Glitch" orders that appear. Completing them earns **Black Market Points** but risks high TOT/Fines.
    *   **Black Market:** Spending Points to hack the system (lower quota) or buy comfort.

**Management systems:**

- Resource management (budget, materials, time)
- Decision-making mechanics
- Automation vs. manual control
- Delegation systems (if applicable)
- Efficiency optimization

### Building and Construction

*   **Core Decision:** **N/A (or Low Priority).**
*   **Rationale:** The game focuses on *player skill* (APM/Accuracy) rather than *system automation*. Buying "Auto-Tapers" would remove the core gameplay loop.
*   **Potential:** "Manual Tools" (e.g., a faster tape gun) could be investigated, but strictly no "Idle Game" automation.

**Construction systems:**

- Placeable objects/structures
- Grid system (free placement, snap-to-grid, tiles)
- Building prerequisites and unlocks
- Upgrade/demolition mechanics
- Space constraints and planning

### Economic and Resource Loops

*   **Resource A: Gold Stars (Corporate Standing)**
    *   **Nature:** Survival Metric (Health). High = Safe, Zero = Fired.
    *   **Source:** Perfect Orders, "Smiling", Daily Quota.
*   **Resource B: Black Market Points (Resistance Crypto)**
    *   **Nature:** Spendable Currency.
    *   **Source:** **Off-Books Orders** (Smuggling requests found in dark web or glitch orders).
    *   **Sink (Black Market):** Buying "Quota Hacks" (Easier day), "TOT Freezes", or localized disruptions.
*   **Loop:** Risk your Safety (Stars/Time) to do Resistance work -> Earn Points -> Buy Hacks to make the Corporate Job survivable.

**Economic design:**

- Income sources
- Expenses and maintenance
- Supply chains (if applicable)
- Market dynamics
- Economic balance and pacing

### Progression and Unlocks

*   **Campaign Structure:** 20 Days fixed.
*   **Metagame:** "Roguelite-ish" replayability. If you fail Day 18, you might need to restart to earn more stars in Day 1-10 to afford the bribes needed for Day 18.
*   **Unlocks:** New Item Tiers (Narrative progression), Black Market access (Day 5+).

**Progression systems:**

- Unlock conditions (achievements, milestones, levels)
- Tech/research tree
- New mechanics/features over time
- Difficulty scaling
- Endgame content

### Sandbox vs. Scenario
*   **Primary Mode:** **Scenario (Story Campaign).** A scripted 20-day narrative descent.
*   **Secondary Modes:** **N/A (Launch).** Sandbox or Endless modes are strictly POST-LAUNCH features to preserve scope.

**Game modes:**

- Scenario/Story Mode (Main focus)
- Sandbox/Endless (Out of Scope for V1)

---

## Progression and Balance

### Player Progression

### Player Progression
*   **Item Mastery:** Players learn complex item names (muscle memory).
*   **Account Upgrades:**
    *   **Gold Stars:** Earned currency.
    *   **Black Market:** Unlocked on Day 5. Allows purchase of "Bribes" (lower quota, slower TOT) or "Comforts" (music tracks).
*   **Narrative Unlocks:** New emails and "Item Tiers" unlock as days progress (Tier 1 Consumer -> Tier 3 War Supplies).

### Difficulty Curve
*   **Ramp:**
    *   **Day 1-4:** Training. Low quota, no modifiers.
    *   **Day 5-10:** "The Squeeze." TOT introduced. Smile Checks start.
    *   **Day 11-15:** "War Economy." Modifiers (Shift/Alt) appear on items. Quotas spike.
    *   **Day 16-20:** "Survival." High complexity, fail-state pressure. Requires Black Market usage to survive.
*   **Pacing:** Wave-based intensity within a single day (Rush Hour vs Lull).

### Economy and Resources
*   **Gold Stars:**
    *   **Earn:** **Lump Sum per Quota Complete** (e.g., +20 Stars). No per-order payout.
    *   **Logic:** Raising the quota makes the game harder (more work for the *same* pay - "Workplace Shrinkflation").
    *   **Spend:** -10 to Remove TOT Strike, -20 to Lower Next Day Quota.

---

## Level Design Framework

### Level Types

### Level Types
*   **The "Day":** Each level is one work shift (approx 5-8 minutes).
*   **Ritual Phase:** "Morning Pledge" minigame intro.
    *   **Progression:** Innocent ("Have a nice day") -> Industrial ("I am a boxing machine") -> Suppressive ("I promise not to bring my political affiliation to work") -> Cultish ("Our great leader can do no wrong").
*   **Work Phase:** The core sorting loop.
*   **Review Phase:** Email check and Black Market shopping.
    *   **Corporate Directory:** Optional screen to view the staff list.
    *   **Progression:** Starts as a normal org chart. Over time, it devolves into a chaotic web of militaristic portraits, "missing" stamps, and nonsensical connections, reflecting the company's descent into madness.

### Level Progression
*   **Linear Campaign:** Days 1 through 20.
*   **Events:** Specific scripted events happen on fixed days (e.g., "Day 6: CEO Announcement - Smiling is Mandatory").
*   **Failure:** Failing a day restarts that Day (with currently held Stars). Use "Roguelite" logic where money persists? (TBD: simpler to just checkpoint start of day).

---

## Art and Audio Direction

### Art Style

### Art Style
*   **Aesthetic:** **90s Corporate Hellscape ("Office Space" Aesthetic).**
*   **Palette:**
    *   **Standard:** Drab Beiges, Faded Greys, and washes of fluorescent white light. The "Soulless Cubicle" look.
    *   **UI:** Windows 95-era grey bevels, deep blue title bars, and clunky serif fonts.
    *   **Danger:** Passive-aggressive bright yellow sticky notes and "PC Load Letter" style system errors.
    *   **Corruption:** The desktop interface glitching not into static, but into "Blue Screens of Death" and melting icons.
*   **UI Focus:** Intentionally retro desktop interface (fake OS), balancing nostalgia with bureaucratic confinement.

### Audio and Music
*   **Dynamic Soundtrack:** Single "Corporate Bossa Nova" track.
    *   **State 1:** Clean, high fidelity.
    *   **State 2:** Slight detuning, vinyl crackle (Stress Low).
    *   **State 3:** Bit-crushed, skipped beats (Stress High).
    *   **State 4:** Harsh noise/drone overlay (Critical/War).
*   **SFX:**
    *   **Good:** Thocky mechanical production keyboard sounds.
    *   **Bad:** Harsh, jarring system buzzers.

---

## Technical Specifications

### Performance Requirements

### Performance Requirements
*   **60 FPS Minimum:** Input latency must be zero. Typing games live or die on responsiveness.
*   **Low Spec Friendly:** Should run on any potato laptop (integrated graphics).

### Platform-Specific Details
*   **PC:** Exclusive focus.
*   **Input:** Raw Keyboard Input handling (scan codes) needed to bypass OS repeat rates for pro typists.

### Asset Requirements
*   **Items:** ~100 Item Definitions (JSON). No 3D models needed (Text only).
*   **Audio:** 1 Adaptive Music System, ~20 UI SFX.
*   **Shaders:** CRT/Glitch shader stack.

---

## Development Epics

### Epic Structure

### Epic Structure
1.  **Core Fulfillment Loop (P0):** Order -> Conveyor -> Box -> Ship. (MVP Complete)
2.  **Oppression Systems (P0):** TOT Timer, Smile Verification, Daily Pledge.
3.  **Economy & Meta (P1):** Gold Stars, Black Market UI, Day Cycle logic.
4.  **Content & Narrative (P1):** Item DB (tiers), Email System, "Glitch" progression.
5.  **Juice & Polish (P2):** CRT Shaders, Camerashake, Audio Manager.

---

## Success Metrics

### Technical Metrics

### Technical Metrics
*   **Input Latency:** < 16ms.
*   **Crash Rate:** 0% (Stable C++ build).

### Gameplay Metrics
*   **WPM (Words Per Minute):** Track average player speed.
*   **Error Rate:** Target < 5% for average player.
*   **Day Completion Rate:** Identifying "quit moments" in the campaign.

---

## Out of Scope

*   **Multiplayer:** Explicitly single-player.
*   **3D Inventory Physics:** Items are abstract text/icons, not physics objects.
*   **Character Customization:** Player is a faceless drone.

---

## Assumptions and Dependencies

*   **User Workspace:** Access to `~/p/pharmasea` was restricted, so architecture follows `prime_pressure` existing patterns (ECS/Raylib).
*   **Music:** Assuming external assets or simple self-made tracks.
*   **Timeline:** 3-month cycle assumes focused solo dev speed.
