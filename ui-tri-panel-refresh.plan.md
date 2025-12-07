# UI Triangle Refresh Plan

## 1) Assets & Constants
- Add CRT bezel/scanline overlays, XP window chrome slices, desk/conveyor stripe textures to `resources/`.
- Define shared layout/color constants for the triangle layout (top computer band, bottom warehouse/boxing split, status bar) in a central header (e.g., `render_views.h` or new `ui_constants`).
- Set nine-slice reference caps for 720p proof: 16–24px corners, 8–12px edges; keep stretchable center sized for 12–16px text.

## 2) Triangle Layout & Styling
- Update render systems (`render_computer.cpp`, `render_warehouse.cpp`, `render_boxing.cpp`) to place panels in a triangle-ish composition: computer band across the top, warehouse/boxing along the bottom with distinct diegetic styling (conveyor for warehouse, workbench for boxing).
- Restyle Computer view as a CRT/XPerience: bezel frame, top nav, and per-order XP-style window cards visible simultaneously in a grid/stack. Make one card clearly focused (scale + brighter frame), keep keyboard selection (numbers/arrow cycle), and show persistent mini progress in the computer band so status is always visible.
- Restyle Warehouse/Boxing to non-terminal diegetic looks: subtle textured backgrounds, dashed drop zones, inline progress badges.
- Base layout at 1280x720: Computer band ~50% height (~360px) when active; ~30% (~215px) when unfocused but still showing cards/progress. Bottom remainder splits roughly 40/20/40 for tri-panel.

## 3) View Focus & Animation
- Implement a slide-in overlay for the Computer band when it is the active view (looks like looking up at the monitor); when unfocused, keep a slimmer bezel/header visible so all panels remain readable. Use one timing: 200–250ms slide, ease-out on enter, ease-in on exit (reuse for badges).
- Preserve keyboard-only navigation cues (focused card highlight, TAB/Shift+TAB hints in status bar).

## 4) Cutscene Mode
- Add a `Cutscene`/`Story` view state and renderer that takes over the full screen with the XP desktop plus popups/emails; reuse window chrome assets and keyboard navigation for advancing/closing popups. While in cutscene, Enter returns to the game; optionally show a minimal "Press Enter to continue" hint.

## 5) Typing Buffer & Status Bar
- Place typing/status bar primarily in the Computer band (XP-style taskbar) with color states (normal, match, error) and inline hints for controls (TAB cycle, numbers select, ENTER clear). Keep a slim status hint elsewhere if needed, but main typing lives in Computer view.

## Phased delivery
- Phase 0 — Assets and constants: import CRT/bezel/window chrome/desk textures; define layout/animation/nine-slice caps and shared colors in `ui_constants.h` (or `render_views.h` if we keep that choice) targeting 1280x720.
- Phase 1 — Layout scaffold: enforce triangle split (computer band 50% active, ~30% unfocused; bottom 40/20/40), keep slim band visible when unfocused, wire base sizing helpers into renderers without full styling.
- Phase 2 — Computer view restyle and focus motion: restyle `RenderComputerView` to the XP/CRT grid with simultaneous cards, focused card highlight (scale + frame), mini progress per order, and slide-in/out tied to ActiveView using shared 200–250ms ease timings.
- Phase 3 — Warehouse/Boxing diegetic styling: apply desk/conveyor textures, dashed drop zones, and inline progress badges across `RenderWarehouseViewBelt`, `RenderWarehouseViewItems`, `RenderWarehouseViewOrderInfo`, and `RenderWarehouseViewBox`; keep slim computer band header readable.
- Phase 4 — Typing and status bar unification: make the computer band the primary typing/status surface (states: normal/match/error; hints for numbers/arrow, TAB/Shift+TAB, ENTER clear) and mirror a slim hint strip in active warehouse/boxing views via `RenderTypingBuffer`.
- Phase 5 — Cutscene mode: add a dedicated ActiveView/state in `ProcessViewSwitch` and a renderer for full-screen XP popups/emails; Enter exits to gameplay with an optional bottom hint.

## Phase checklists
- Phase 0
  - [x] Add CRT/bezel/window chrome and desk/conveyor textures with 16–24px corner, 8–12px edge nine-slice guidance
  - [x] Define layout, color, and animation constants at 1280x720 (heights, 40/20/40 splits, 200–250ms slide timing)
- Phase 1
  - [x] Apply triangle layout sizing across renderers and keep slim unfocused computer band visible
  - [x] Centralize helpers for band heights and splits so views share the same math
- Phase 2
  - [x] Restyle `RenderComputerView` to simultaneous cards with focus highlight and mini progress
  - [x] Hook slide-in/out animation to ActiveView using shared timing/ease and keep TAB/Shift+TAB/number cues visible
- Phase 3
  - [x] Style warehouse/boxing backgrounds with textures, dashed drop zones, and inline progress badges across the listed renderers
  - [x] Verify 40/20/40 split coexists with the always-visible slim computer header
- Phase 4
  - [ ] Make typing/status live primarily in the computer band with state colors and control hints
  - [ ] Mirror a slim hint strip in active warehouse/boxing views without duplicating the full bar
- Phase 5
  - [ ] Add cutscene/Story ActiveView and renderer for full-screen XP popups/emails
  - [ ] Ensure Enter cleanly returns to gameplay and optionally shows a minimal hint

## Multiple-Choice Clarifications
1) Asset packaging (filenames/slices/scaling)
   - A: Provide exact filenames + target px sizes + 9-slice in one atlas
   - B: Provide individual files; we infer sizes and slice guides
   - C: Minimal spec: rough dimensions only; we trial-and-error in code
   - Chosen: A (scripted spritesheet with 9-slice; labels already in resources)

2) Shared constants home
   - A: New `ui_constants.h` for layout/colors shared by renderers
   - B: Keep in `render_views.h` alongside register functions
   - C: Separate headers per view (computer/warehouse/boxing)
   - Chosen: B (keep simple now; refactor later)

3) Base resolution and scaling
   - A: Lock to a logical canvas (e.g., 1280x720) and scale to window
   - B: Fluid layout in percentages; no fixed base resolution
   - C: Fixed pixel layout; no dynamic scaling
   - Chosen: A (target 1280x720 proof; scale later)

4) Triangle composition split
   - A: Top band ~50% (active), ~30% when unfocused; bottom remainder split 40/20/40
   - B: Equal thirds stacked; triangle is purely decorative overlay
   - C: Skewed/rotated geometry (true triangle), requiring sheared draws
   - Chosen: A

5) Computer card layout
   - A: Grid with 2–3 columns; scroll when overflow
   - B: Single-column vertical stack; scroll when overflow
   - C: Carousel/one-at-a-time with nav controls
   - Chosen: A (simultaneous cards; one clearly focused)

6) Persistent band content when unfocused
   - A: Show mini progress bars per order + typing hint strip
   - B: Show only current selection + minimal status text
   - C: Hide all dynamic info; bezel only when unfocused
   - Tentative: A (mini progress bars + typing hint strip)

7) Slide-in focus behavior
   - A: Active view triggers slide-in/out; state tied to ActiveView
   - B: Separate focus state; can be focused without being ActiveView
   - C: No slide animation; just swap bezels (static change)
   - Tentative: A (slide tied to ActiveView; revisit if feel is off)

8) Animation timing/easing
   - A: 200–250 ms ease-out cubic for slide and badges
   - B: 350–450 ms ease-in-out for a softer feel
   - C: No animations; static transitions only
   - Chosen: A (single timing for slide/badges; ease-out enter, ease-in exit)

9) Typing/status bar mirroring when in Warehouse/Boxing
   - A: Primary bar stays in computer band; slim mirrored hint in active view
   - B: Bar fully relocates to active view; none in computer band
   - C: Duplicate full bar in both places while other views are active
   - Chosen: A

10) Cutscene integration with ActiveView
    - A: New enum state; cutscene suppresses other renderers entirely
    - B: Overlay on top of existing layout; underlying views still render dimmed
    - C: Runs inside the computer band only; warehouse/boxing remain visible
     - Chosen: A
