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

### Phase 2: MVP (Order & Warehouse Screens) ✅
1. ✅ Create basic components (Order, TypingBuffer, SelectedOrder, ActiveView)
2. ✅ Implement order selection system: ProcessOrderSelection system, number key handling (no ENTER needed)
   - ✅ Only works when no order is currently selected
   - TODO: Allow switching orders mid-way
   - TODO: Allow deselecting orders
3. ✅ Implement view switching: ActiveView component, ProcessViewSwitch system, TAB/Shift+TAB key handling
   - ✅ Bidirectional view switching (TAB forward, Shift+TAB backward)
   - ✅ Clear typing buffer when switching views
   - ✅ Three views: Computer, Warehouse, Boxing
4. ✅ Implement typing input system: TypingBuffer component, ProcessTypingInput system, keyboard input handling
   - ✅ Case-insensitive matching
   - ✅ Match as you type (immediate feedback)
   - ✅ ENTER key clears buffer manually
   - ✅ Number keys excluded from typing buffer
   - TODO: Red error state when no possible matches remain
   - TODO: Visual error state for wrong items
5. ✅ Implement item selection tracking: MatchItemToOrder system, mark items as selected when typed
   - ✅ Items can be collected in any order
   - ✅ Duplicate items require multiple typing (one per instance)
   - ✅ Only matches against selected order
   - ✅ Tracks selected items in Order.selected_items
6. TODO: Implement order completion: ProcessOrderCompletion system, mark order complete when all items selected
   - TODO: Order completion animation/feedback
7. ✅ Implement order generation: GenerateOrders system, spawn orders every 1 second (testing)
   - ✅ Only spawns if fewer than 3 active orders
   - ✅ Maximum 3 active orders
   - TODO: Change interval to 90 seconds
8. ✅ Create basic rendering systems: RenderComputerView, RenderWarehouseView, RenderBoxingView, RenderTypingBuffer
   - ✅ Terminal-style UI with borders
   - ✅ Black background, green headers
   - ✅ Typing buffer always visible at bottom
9. ✅ Visual feedback: highlight selected orders (green), highlight selected items (green/yellow with checkmark)
   - ✅ Selected orders highlighted in green on Computer view
   - ✅ Selected items show checkmark and color coding on Warehouse view
   - TODO: Red typing buffer on error

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

## Gameplay Loop

### Core Flow (Phase 2 MVP - Order & Warehouse Screens)

1. **Order Screen**: Orders appear on screen
   - Player sees list of orders with numbers (1, 2, 3, etc.)
   - Player presses a number key (1-9) then ENTER to select an order
   - Selected order turns green/highlighted
   - Only one order can be selected at a time
   - Order shows list of items needed

2. **Warehouse Screen**: Type items from selected order
   - Player presses TAB to switch to warehouse screen
   - Player types item names from the selected order
   - Each item typed gets marked as selected (turns green in the order list)
   - Visual feedback shows which items have been selected
   - Continue typing until all items in the order are selected

3. **Complete Order**: Mark order as finished
   - Once all items in order list are fully selected, return to Order Screen
   - Press the same number key again to mark the order as complete
   - Order is removed or marked as completed

**Note**: Boxing screen will be implemented later. Focus is on Order and Warehouse screens for Phase 2 MVP.

### Detailed Mechanics

**Order Selection:**
- Orders are numbered by position (1st = 1, 2nd = 2, 3rd = 3)
- Press number key (1-9) to select an order (no ENTER needed)
- Only one order can be selected at a time
- Only works when no order is currently selected
- TODO: Allow switching orders mid-way
- TODO: Allow deselecting orders (ESC key or similar)

**Item Collection:**
- Items can be collected in any order (no specific sequence required)
- Orders can have duplicate items (e.g., "book, book, pen")
- Typing an item name selects one instance of that item
- For duplicates, must type the item name multiple times (once per instance)
- Case-insensitive matching (typing "BOOK" or "book" both work)

**Typing Mechanics:**
- Match as you type (immediate feedback, no need to press ENTER)
- Typing buffer clears when pressing TAB to switch views
- Press ENTER at any time to manually clear the typing buffer
- Typing buffer turns RED when no possible matching items remain in the current order
- TODO: Visual error state for wrong items (items not in order, already selected)

**Order Completion:**
- Once all items in an order are selected, return to Order Screen
- Press the same number key again to mark order as complete
- TODO: Order completion animation/feedback
- Completed orders are removed from active list

**Order Generation:**
- New orders spawn automatically every 90 seconds
- Only spawns if there are fewer than 3 active orders
- Maximum of 3 active orders at a time

**View Behavior:**
- Computer Screen: Number keys work for order selection (when no order selected)
- Warehouse Screen: Typing input matches against selected order's items
- Boxing Screen: Placeholder (coming soon)
- Typing buffer is always visible but context-sensitive
- Press TAB to cycle forward (Computer → Warehouse → Boxing → Computer)
- Press Shift+TAB to cycle backward (Computer → Boxing → Warehouse → Computer)
- Press ESC to close the game

## Visual Design & Layout

### Screen Layout (Phase 2 MVP)

**90s Terminal/Component Screen Aesthetic**

The game uses a view-switching system where the player switches between three views using TAB:
1. **Computer Screen** - Shows orders, select with number keys
2. **Warehouse Screen** - Type items from selected order
3. **Boxing Screen** - Placeholder for future functionality

**View Switching:**
- Press TAB to switch between Order ↔ Warehouse
- Typing input is context-sensitive based on current view
- Typing buffer is always visible at the bottom regardless of view

**Layout:**

**Computer Screen:**
```
┌─────────────────────────────────────────────────────────┐
│  [COMPUTER]  [WAREHOUSE]  [BOXING]                    │
│  (Press TAB to switch)                                 │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ > COMPUTER SCREEN                                │  │
│  │                                                  │  │
│  │  1. book, pen, mug                              │  │
│  │  2. cup, bag                                    │  │
│  │  3. toy, hat                                    │  │
│  │                                                  │  │
│  │  [Press number key to select order]              │  │
│  │  Selected order turns green                     │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ TYPING:                                          │  │
│  │ (Always visible)                                 │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

**Warehouse Screen:**
```
┌─────────────────────────────────────────────────────────┐
│  [COMPUTER]  [WAREHOUSE]  [BOXING]                    │
│  (Press TAB to switch)                                 │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ > WAREHOUSE SCREEN                                │  │
│  │                                                  │  │
│  │  Selected Order: 1                               │  │
│  │  Items needed:                                   │  │
│  │    book ✓ (green)                                │  │
│  │    pen                                           │  │
│  │    mug ✓ (green)                                 │  │
│  │                                                  │  │
│  │  [Type item names from order]                    │  │
│  │  [Each typed item turns green when selected]     │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │ TYPING: book                                     │  │
│  │ (Always visible)                                 │  │
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
- Two distinct views that player switches between with TAB
- Only one view visible at a time (full screen)
- Terminal/console aesthetic with monospace font
- Green text on black background (classic 90s terminal look)

**Computer Screen:**
- Shows list of active orders with numbers (1, 2, 3, etc.)
- Each order displays: order number, list of items needed
- Player selects order by pressing number key (1-9) (no ENTER needed)
- Selected order turns green/highlighted
- Only one order can be selected at a time
- Only works when no order is currently selected
- Shows which order is currently selected
- Terminal-style header: "> COMPUTER SCREEN"
- Simple text list format
- Instructions: "[Press number key to select order]"

**Warehouse Screen:**
- Shows the currently selected order and its items
- Displays which items have been selected (marked with ✓ and green color)
- Player types item names from the selected order
- Items can be collected in any order
- Each item typed gets marked as selected (turns green in the list)
- For duplicate items, must type the item name multiple times (once per instance)
- Visual feedback: selected items show checkmark and green color
- Terminal-style header: "> WAREHOUSE SCREEN"
- Shows order number at top: "Selected Order: 1"
- Instructions: "[Type item names from order]"
- Match as you type (immediate feedback, no ENTER needed)

**Typing Buffer (Always Visible):**
- Fixed position at bottom of screen
- Shows current typing input: "TYPING: book"
- Always visible regardless of current view
- Context-sensitive: typing affects items in current view
- Green/yellow text for normal typing
- Red text when no possible matches remain
- Clears when pressing TAB to switch views
- Press ENTER to manually clear buffer
- Terminal-style box border

### Rendering Approach

**Phase 2 MVP:**
- Terminal/console aesthetic with monospace font
- Text-based rendering (simple and fast)
- Use raylib::DrawText for all elements
- Color scheme:
  - Green: headers, active text, matched items, selected orders/items
  - White/Gray: normal text, item lists
  - Yellow: typing buffer (normal state)
  - Red: typing buffer (error state - no possible matches)
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
1. **ActiveView Component**: Tracks current view (Order/Warehouse)
2. **ProcessViewSwitch**: System to handle TAB key for view switching
3. **RenderCurrentView**: System that renders the appropriate view based on ActiveView

**Order Selection:**
1. **SelectedOrder Component**: Tracks which order is currently selected (stores order number/ID)
2. **ProcessOrderSelection**: System to handle number key + ENTER for order selection
3. **Order Component**: Contains order data (order number, list of items, completion status)

**Item Selection Tracking:**
1. **OrderItem Component**: Represents an item in an order
2. **IsSelected Component**: Tag component marking an item as selected
3. **ProcessItemSelection**: System to mark items as selected when typed in warehouse view

**View-Specific Rendering:**
1. **RenderOrderView**: Shows orders list (only when ActiveView == Order)
   - Renders all orders with numbers
   - Highlights selected order in green
   - Shows selection instructions
2. **RenderWarehouseView**: Shows selected order and items (only when ActiveView == Warehouse)
   - Shows selected order number
   - Renders order items list
   - Highlights selected items in green with checkmark
   - Shows typing instructions
3. **RenderTypingBuffer**: Always renders typing buffer at bottom (regardless of view)

**Input Handling:**
1. **ProcessOrderSelection**: Handles number key (1-9) + ENTER to select order
   - Sets SelectedOrder component
   - Only one order selected at a time
   - TODO: Prevent switching orders mid-way (for now, block switching)
   - TODO: Allow deselecting orders
2. **ProcessTypingInput**: Context-sensitive based on ActiveView
   - Order view: number keys + ENTER work for order selection
   - Warehouse view: matches typed items against selected order's items (as you type)
   - Case-insensitive matching
   - When item matches, adds IsSelected tag to one instance of that item
   - For duplicate items, must type multiple times
   - Clears buffer when TAB is pressed
   - ENTER key clears buffer manually
   - Sets error state (red) when no possible matches remain
3. **ProcessViewSwitch**: Handles TAB key to switch between Order ↔ Warehouse
   - Clears typing buffer when switching views
4. **ProcessOrderCompletion**: Handles number key press when all items selected
   - When order is fully selected and player presses number again, marks order complete
   - TODO: Add completion animation/feedback
5. **ProcessOrderGeneration**: Spawns new orders every 90 seconds
   - Only spawns if fewer than 3 active orders
   - Maximum 3 active orders at a time

### Layout Constants

```cpp
// View states
enum struct ViewState {
  Order,
  Warehouse
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

