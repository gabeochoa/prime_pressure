# Game Brief: prime_pressure

**Date:** 2025-12-14
**Author:** gabe
**Status:** Draft for GDD Development

---

## Executive Summary

*Prime Pressure* is a satirical typing-based management game where players fulfill orders in a retro-futuristic warehouse. It combines the high-skill frantic gameplay of *Cook, Serve, Delicious!* with the oppressive narrative choices of *Papers, Please*. Players must balance fulfillment quotas, dystopian "Smile Checks," and an increasingly disturbing item catalog as the world outside shifts from mundane commerce to a war economy. The goal is to create a "stress-fun" experience that critiques the human cost of convenience.

---

## Game Vision

### Core Concept

A satirical warehouse fulfillment game where you type to fill orders while navigating an increasingly dystopian corporate environment that shifts from "business as usual" to a "war economy."

### Elevator Pitch

*Prime Pressure* is a typing-based simulation game set in a retro-futuristic fulfillment center. What starts as a simple job boxing harmless consumer goods slowly evolves into a high-stakes struggle against oppressive quotas and moral decay as the world outside descends into chaos. Will you meet your metrics, or will you become just another glich in the system?

### Vision Statement

To create a visceral, "stress-fun" experience that uses typing mechanics to critique the human cost of modern convenience, blending the frantic gameplay of *Cook, Serve, Delicious!* with the narrative oppression of *Papers, Please*.

---

## Target Market

### Primary Audience

Fans of "job simulator" games with deep narrative layers (*Papers, Please*, *Not Tonight*) and high-skill typing/management games (*Cook, Serve, Delicious!*).

### Secondary Audience

Players interested in political satire, dystopian narratives, and retro-terminal aesthetics.

### Market Context

The "workplace dystopia" genre has a dedicated following. Players enjoy the tension between mastering complex mechanics and navigating a hostile narrative setting. *Prime Pressure* capitalizes on this by gamifying the familiar (and often criticized) experience of warehouse fulfillment.

### Elevator Pitch

*Prime Pressure* is a typing-based simulation game set in a retro-futuristic fulfillment center. What starts as a simple job boxing harmless consumer goods slowly evolves into a high-stakes struggle against oppressive quotas and moral decay as the world outside descends into chaos. Will you meet your metrics, or will you become just another glich in the system?

### Vision Statement

To create a visceral, "stress-fun" experience that uses typing mechanics to critique the human cost of modern convenience, blending the frantic gameplay of *Cook, Serve, Delicious!* with the narrative oppression of *Papers, Please*.

---

## Target Market

### Primary Audience

Fans of "job simulator" games with deep narrative layers (*Papers, Please*, *Not Tonight*) and high-skill typing/management games (*Cook, Serve, Delicious!*).

### Secondary Audience

Players interested in political satire, dystopian narratives, and retro-terminal aesthetics.

### Market Context

The "workplace dystopia" genre has a dedicated following. Players enjoy the tension between mastering complex mechanics and navigating a hostile narrative setting. *Prime Pressure* capitalizes on this by gamifying the familiar (and often criticized) experience of warehouse fulfillment.

---

## Game Fundamentals

### Core Gameplay Pillars

1.  **Narrative Action:** Mechanical inputs (typing) are directly tied to narrative tension. Complex inputs represent Dangerous items.
2.  **Satire via Systems:** The horror of the setting is conveyed through "Corporate Benevolence" mechanics (Smile Checks) that interrupt flow.
3.  **The Boiling Frog:** The game starts normal and slowly descends into dystopia without the UI ever acknowledging the horror.
4.  **Rhythmic Flow:** Mastery of the typing rhythm provides the "fun" counterweight to the narrative "stress."

### Primary Mechanics

*   **Complex Typing:** Type item names to box them. Modifiers (e.g., `[SHIFT] + [!]`) are added for hazardous/narrative items.
*   **Time Off Task (TOT):** A passive timer that punishes inactivity, forcing players to rush even when conveyors are slow.
*   **Smile Verification:** A "Wellness Check" popup that requires a specific keystroke to "smile" (compliance) to continue working.
*   **The Morning Pledge:** A daily typing ritual/minigame where players type corporate slogans to "unlock" their shift.
*   **View Switching:** Tab between Computer (Orders), Warehouse (Fulfillment), and Boxing (Shipping) views.

### Player Experience Goals

*   **Flow / Competence:** Feeling like a master of the machine (hitting a perfect typing streak).
*   **Cognitive Dissonance:** The unease of typing "Happy" while shipping "Riot Shields."
*   **Oppression / Stress:** The pressure of the TOT timer and arbitrary management interruptions.
*   **Curiosity:** "What does this item mean?" (Environmental storytelling through the item database).

---

## Scope and Constraints

### Target Platforms

*   **Primary:** PC (Steam, Itch.io) - Keyboard is essential.
*   **Secondary:** Mac / Linux (Native support pending Raylib checks).
*   **Excluded:** Mobile/Console (Typing mechanic is not portable).

### Development Timeline

*   **Phase 1 (Done):** Core Engine & MVP Mechanics (Order/Warehouse views).
*   **Phase 2 (1 Month):** "Pressure" Update (TOT, Smile System, Audio Manager).
*   **Phase 3 (1 Month):** Content Production (Item Database writers, Music assets).
*   **Phase 4 (1 Month):** Polish & Narrative Scripting (The "War Arc").
*   **Target Release:** Vertical Slice in 3 months.

### Budget Considerations

*   **Low Budget / Indie:**
    *   Assets: Asset store or self-made (Pixel art/Terminal UI).
    *   Audio: Potential outsourcing for dynamic/glitch music if internal skills insufficient.
    *   Marketing: Community-driven (Itch.io devlogs).

### Team Resources

*   **Core Team:** 1 (Solo Developer - Gabe).
*   **Current Skills:** C++, Raylib, Game Design.
*   **Gaps:** Sound Design (Adaptive audio), Narrative Writing (Scaling item descriptions).

### Technical Constraints

### Technical Constraints

*   **Engine:** Custom C++ / Raylib with `afterhours` ECS (Entity Component System).
*   **Architecture:** Heavily system-driven (`order_state_machine.cpp`, `MatchItemToOrderSystem`).
*   **Narrative Integration:** Must integrate narrative events (Smile Checks, Emails) without breaking the strict `OrderWorkflow` state machine; likely requires new "Interrupt" states or parallel systems.
*   **Audio:** Currently minimal; needs a robust audio manager for the "glitching" music progression.

---

## Reference Framework

### Inspiration Games

*   ***Cook, Serve, Delicious!***: The frantic typing flow and "zen" state of high-speed execution.
*   ***Papers, Please***: The drag of bureaucracy and the moral weight of distinct items/people.
*   ***Office Space*** (Film): The tone of corporate absurdity and "fake happiness."
*   ***Wilmot's Warehouse***: The aesthetic of organizing chaos.

### Competitive Analysis

*   **Pure Typing Games (Typing of the Dead, Epistory):** Often lack management depth or satirical bite.
*   **Warehouse Simulators (Wilmot's):** Focus on spatial puzzles, not the *human/political* cost.
*   **Job Sims (Job Simulator):** Often purely comedic/slapstick. *Prime Pressure* leans into dark satire/horror.

### Key Differentiators

1.  **Mechanics as Narrative:** The typing gets harder *because* the world gets worse (Modifiers), not just arbitrary difficulty.
2.  **The "Glitch" Aesthetic:** The UI and Music degrade over time, mirroring the protagonist's mental state.
3.  **Satire of "Now":** Directly addresses the current cultural conversation around fulfillment centers and labor rights.

---

## Content Framework

### World and Setting

*   **Place:** A fulfillment center for a megacorp ("The Company"). Sterile, neon-lit, windowless.
*   **Time:** Near-future / Alternate Present.
*   **The World Outside:** Starts normal, descends into civil unrest/war. We only see it through the *items* we ship (e.g., "Protest Sign" -> "Riot Shield" -> "MRE").

### Narrative Approach

*   **Emergent/Environmental:** The story is told primarily through the *Item Database* and *Email Notifications*.
*   **Show, Don't Tell:** No cutscenes. The conveyer belt tells the story.
*   **The Morning Pledge:** A daily framing device that establishes the company's escalating propaganda.

### Content Volume

*   **Campaign Length:** ~20 in-game days (approx. 2-3 hours for complete run).
*   **Items:** ~100 unique items across 3 tiers (Consumer, Sadness, Dystopia).
*   **Music:** 1 dynamic track with 3-4 progressive states (Clean -> Glitch -> Noise).

---

## Art and Audio Direction

### Visual Style

*   **Aesthetic:** 90s CRT Terminal / DOS interface. High contrast Green/Black.
*   **Evolution:** The screen starts clean. Over time, dead pixels, screen tearing, and "invading" red warning overlays increase.
*   **Font:** Monospaced, legible (critical for typing gameplay).

### Audio Style

*   **Music:** "Elevator Bossa Nova" that slowly corrupted by bit-crushing and distortion.
*   **SFX:** Satisfying mechanical keyboard clacks (ASMR quality). Harsh buzzers for errors/TOT warnings.
*   **Voice:** Minimal or gibberish (manager drone).

### Production Approach

*   **Code First:** Leveraging the existing C++/Raylib engine.
*   **Procedural Content:** Using data tables (JSON) for items to make content scaling easier.
*   **Iterative Polish:** Focus on game feel (cam shake, hit stop) early, as it drives the "fun."

---

## Risk Assessment

### Key Risks

*   **Tone Balance:** Risk of being too depressing/preachy and losing the "fun."
*   **Repetition:** Typing words can get boring if modifiers don't add enough tactical depth.
*   **Scope Creep:** Trying to tell too complex a story within a simple typing game.

### Technical Challenges

*   **State Machine Complexity:** Ensuring the "Smile Check" interrupt resumes the `OrderWorkflow` state perfectly without soft-locking the order.
*   **Audio Sync:** Getting music glitches to sync with gameplay stress levels (likely custom audio manager needed).
*   **Input Handling:** Differentiating "Game Commands" (Tab) from "Typing Inputs" (Item Names) cleanly, especially when Modifiers are introduced.

### Market Risks

*   **Niche Appeal:** Hardcore typing games are a specific niche.
*   **Political Backlash:** Overt commentary on labor can alienate some players (though likely attracts the target indie audience).

### Mitigation Strategies

*   **Fun First:** Prioritize satisfying feedback (juice) so the *act* of playing is fun, even if the *context* is sad.
*   **Humor:** Use the "Office Space" absurdity to cut the tension.
*   **Playtesting:** Early testing of the "Modifier" system to ensure it limits APM (Actions Per Minute) fairly without feeling broken.

---

## Success Criteria

### MVP Definition

*   **Core Loop:** Order Selection -> Type to Box -> Ship.
*   **Pressure:** Functioning TOT Timer and Smile Check.
*   **Content:** Day 1-5 loop with Tier 1 (Honeymoon) items only.
*   **Win/Loss:** Basic quota tracking.

### Success Metrics

*   **Retention:** Players completing the full 20-day cycle.
*   **Engagement:** "Flow state" achievement (players reporting losing track of time).
*   **Narrative Impact:** Players noticing the item shift (Tier 1 -> Tier 3) without being told explicitly.

### Launch Goals

*   **Itch.io Demo:** Release Day 1-5 demo for feedback.
*   **Community:** Build a small Discord community around "High Score" runs.

---

## Next Steps

### Immediate Actions

1.  **Prototype TOT & Smile:** Implement the two core "Oppression" mechanics in the current ECS.
2.  **Item Database:** Draft the JSON structure for Tier 1-3 items with modifiers.
3.  **Audio System:** Implement a basic audio manager to handle the music/SFX layering.

### Research Needs

*   **Real-world Warehouse Horror Stories:** Collect anecdotes for "Email" content flavor.
*   **Audio References:** Listen to tracks that effectively use "glitch" for tension (e.g., *Doki Doki Literature Club*, *Pony Island*).

### Open Questions

*   **Modifier Complexity:** How difficult should "Dangerous" items be? Do we use Shift, Alt, or just weird character combos?
*   **Fail State:** What happens when you get fired? Game Over, or "Demotion" (Harder difficulty)?

---

## Appendices

### A. Research Summary

*   **Source:** Brainstorming Session (2025-12-14).
*   **Key Insight:** "Mechanics are Narrative." constraints > cutscenes.
*   **Political Theme:** Human cost of convenience.

### B. Stakeholder Input

*   **User (Gabe):** Emphasized specific desire for "Office Space" satire vs. "War Economy" reality.
*   **Constraint:** Solo dev context key to scope decisions.

### C. References

*   `idea.md`: Project technical plan and Phase 2 status.
*   `brainstorming-session-2025-12-14.md`: Narrative & Progression deep dive.
*   `src/`: Existing ECS architecture reference.

---

_This Game Brief serves as the foundational input for Game Design Document (GDD) creation._

_Next Steps: Use the `workflow gdd` command to create detailed game design documentation._
