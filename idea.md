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

## Technical Notes
- Uses afterhours ECS library (git submodule)
- Keyboard letter assets copied from my_name_chef
- Basic window rendering working
- Project builds successfully

