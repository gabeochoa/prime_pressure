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

## To-dos
- [ ] Add CRT/bezel/window chrome assets and UI constants
- [ ] Restyle renderers to triangle layout and XP/desk themes
- [ ] Add computer slide-in focus animation and cues
- [ ] Implement full-screen cutscene XP popup renderer
- [ ] Unify typing/status bar with states and hints

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
