# Prime Pressure - Project Setup Plan

## Overview
A typing-based game similar to Cook Serve Delicious, set in an Amazon warehouse. Players grab items from a list by typing their names and box them for customers. The game starts simple and progressively becomes more difficult, eventually evolving into dystopian commentary about the free shipping industry.

## Development Phases

### Phase 0: Repository Setup ✅
1. ✅ Create project directory structure (src/, resources/, output/, vendor/)
2. ✅ Copy and adapt makefile from my_name_chef
3. ✅ Set up vendor/afterhours as git submodule
4. ✅ Copy resources from other games (keyboard letter assets)
5. ✅ Create PROJECT_RULES.md based on my_name_chef pattern
6. ✅ Create placeholder files (main.cpp, game.h, preload.h, settings.h) with minimal structure
7. ✅ Verify project builds and runs (basic window)

### Phase 1: Basic Window ✅
1. ✅ Implement main.cpp with basic entry point, command line parsing
2. ✅ Set up preload.cpp with window initialization, raylib setup
3. ✅ Create basic game loop in game.cpp, initialize SystemManager
4. ✅ Basic render loop (clear screen, present)
5. ✅ Verify window opens and closes properly

### Phase 2: MVP (Simple Typing)
1. Create basic components (Order, Item, TypingBuffer, Box, OrderQueue)
2. Implement typing input system: TypingBuffer component, ProcessTypingInput system, keyboard input handling
3. Implement item matching system: MatchItemToOrder system, case-insensitive matching
4. Implement grab and box mechanics: GrabItem system, BoxItem system, item state management
5. Create basic rendering systems: RenderOrders, RenderTypingBuffer, RenderBox, simple UI layout

### Phase 3: Pressure Systems
1. Add timer system: Timer component, UpdateTimers system, order timeout handling, RenderTimer UI
2. Add quota system: Quota component, CheckQuota system, daily/hourly quotas, RenderQuota UI
3. Score tracking
4. Order timeout handling

### Phase 4: Complexity
1. Create item database: JSON file with items, categories, complexity levels, name variations
2. Implement progressive difficulty: complex items, special handling, multiple simultaneous orders
3. Add typing shortcuts system: evolve from full names to shortcuts, shortcut unlocking mechanism

### Phase 5: Commentary
1. Implement dystopian commentary elements: fatigue system, escalating demands, ethical dilemmas, narrative events
2. Environmental storytelling
3. Narrative text system

## Game Name
**Prime Pressure**

## Visual Design & Layout

### Screen Layout (Phase 2 MVP)

**90s Terminal/Component Screen Aesthetic**

The game uses a view-switching system where the player switches between three views using TAB:
1. **Computer Screen (Left)** - Shows orders
2. **Shelf View (Center)** - Shows available items
3. **Box View (Right)** - Shows packed items

**View Switching:**
- Press TAB to cycle through views: Computer → Shelf → Box → Computer
- Typing input is context-sensitive based on current view
- Typing buffer is always visible at the bottom regardless of view

**Layout:**

```
┌─────────────────────────────────────────────────────────┐
│  [COMPUTER VIEW]  [SHELF VIEW]  [BOX VIEW]            │
│  (Press TAB to switch)                                 │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ > COMPUTER VIEW                                  │  │
│  │                                                  │  │
│  │  Order 1: book, pen, mug                        │  │
│  │  Order 2: cup, bag                              │  │
│  │  Order 3: toy, hat                              │  │
│  │                                                  │  │
│  │  [Type item names to match orders]              │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ > SHELF VIEW                                      │  │
│  │                                                  │  │
│  │  Available Items:                                │  │
│  │  [book] [pen] [mug] [cup] [bag]                 │  │
│  │  [box]  [toy] [hat] [key] [map]                 │  │
│  │                                                  │  │
│  │  [Type item name to grab from shelf]            │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ > BOX VIEW                                        │  │
│  │                                                  │  │
│  │  Box: 3/10                                       │  │
│  │  [book] [pen] [mug]                              │  │
│  │                                                  │  │
│  │  [Type item name to pack into box]              │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ TYPING: book                                     │  │
│  │ (Always visible, context-sensitive)              │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

**Visual Style:**
- Monospace font (terminal/console style)
- Green text on black background (classic terminal)
- Simple ASCII borders/boxes
- Minimalist, functional aesthetic
- Text-based rendering (no sprites initially)

### Visual Elements

**View System:**
- Three distinct views that player switches between with TAB
- Only one view visible at a time (full screen)
- Terminal/console aesthetic with monospace font
- Green text on black background (classic 90s terminal look)

**Computer View (Left):**
- Shows list of active orders
- Each order displays: order number, list of items needed
- Typing matches items in orders (for grabbing)
- Terminal-style header: "> COMPUTER VIEW"
- Simple text list format

**Shelf View (Center):**
- Shows all available items on shelves
- Grid/list of item names in brackets: [book] [pen] [mug]...
- Typing matches items on shelf (for grabbing)
- Visual feedback when typing matches an item (highlight in green)
- Terminal-style header: "> SHELF VIEW"

**Box View (Right):**
- Shows current box contents
- Displays capacity: "Box: 3/10"
- List of packed items: [book] [pen] [mug]
- Typing matches items in hand/grabbed state (for packing)
- Terminal-style header: "> BOX VIEW"

**Typing Buffer (Always Visible):**
- Fixed position at bottom of screen
- Shows current typing input: "TYPING: book"
- Always visible regardless of current view
- Context-sensitive: typing affects items in current view
- Green/yellow text for visibility
- Terminal-style box border

### Rendering Approach

**Phase 2 MVP:**
- Terminal/console aesthetic with monospace font
- Text-based rendering (simple and fast)
- Use raylib::DrawText for all elements
- Color scheme:
  - Green: headers, active text, matched items
  - White/Gray: normal text, item lists
  - Yellow: typing buffer
  - Black background (terminal style)
- ASCII-style borders/boxes using characters or simple rectangles
- View switching animation (optional: fade or slide)

**Future Enhancements:**
- More terminal effects (cursor blink, scanlines)
- Item sprites/icons (optional, keep terminal aesthetic)
- Sound effects (terminal beeps, keyboard clicks)
- Animations (text appearing, view transitions)
- Visual keyboard showing pressed keys (using keyboard letter assets)
- More sophisticated terminal UI elements (progress bars, etc.)

### Component Rendering Strategy

**View Management:**
1. **ActiveView Component**: Tracks current view (Computer/Shelf/Box)
2. **ProcessViewSwitch**: System to handle TAB key for view switching
3. **RenderCurrentView**: System that renders the appropriate view based on ActiveView

**View-Specific Rendering:**
1. **RenderComputerView**: Shows orders list (only when ActiveView == Computer)
2. **RenderShelfView**: Shows available items on shelves (only when ActiveView == Shelf)
3. **RenderBoxView**: Shows box contents (only when ActiveView == Box)
4. **RenderTypingBuffer**: Always renders typing buffer at bottom (regardless of view)

**Input Handling:**
1. **ProcessTypingInput**: Context-sensitive based on ActiveView
   - Computer view: matches against order items
   - Shelf view: matches against shelf items
   - Box view: matches against grabbed items (for packing)
2. **ProcessViewSwitch**: Handles TAB key to cycle through views

### Layout Constants

```cpp
// View states
enum struct ViewState {
  Computer,
  Shelf,
  Box
};

// Typing buffer (always visible)
const float TYPING_BUFFER_X = 20.0f;
const float TYPING_BUFFER_Y = screen_height - 60.0f;
const float TYPING_BUFFER_WIDTH = screen_width - 40.0f;

// View content area (full screen minus typing buffer)
const float VIEW_X = 20.0f;
const float VIEW_Y = 20.0f;
const float VIEW_WIDTH = screen_width - 40.0f;
const float VIEW_HEIGHT = screen_height - 100.0f;

// Terminal-style colors
const raylib::Color TERMINAL_GREEN = raylib::Color{0, 255, 0, 255};
const raylib::Color TERMINAL_YELLOW = raylib::Color{255, 255, 0, 255};
const raylib::Color TERMINAL_WHITE = raylib::Color{255, 255, 255, 255};
const raylib::Color TERMINAL_GRAY = raylib::Color{128, 128, 128, 255};
const raylib::Color TERMINAL_BLACK = raylib::Color{0, 0, 0, 255};
```

## Technical Notes
- Uses afterhours ECS library (git submodule)
- Keyboard letter assets copied from my_name_chef
- Basic window rendering working
- Project builds successfully

