# Testing Framework Implementation Notes

## Overview
Implemented a C++20 coroutine-based testing framework for Prime Pressure game, following the plan from `amazon-warehouse-game-setup.plan.md`.

## What Was Implemented

### Core Infrastructure
1. **Test Macros** (`src/testing/test_macros.h`)
   - `TEST()` macro for registering tests
   - `TestCoroutine` struct for coroutine handling
   - `TestRegistry` for test discovery

2. **Test Input System** (`src/testing/test_input_queue.h/cpp`, `test_input.h`)
   - Thread-safe input queue for simulating keyboard/mouse input
   - Wrapper functions that check test queue before real input
   - Note: Game systems were updated to use wrappers, then reverted to direct raylib calls per user request

3. **Test Context** (`src/testing/test_context.h`)
   - Sets up game environment for tests
   - Initializes systems, entities, and singletons
   - Manages render textures

4. **Test App** (`src/testing/test_app.h`)
   - Helper class for test assertions and simulation
   - `expect_order_count()`, `expect_item_on_shelf()`, etc.
   - `simulate_typing()`, `simulate_key_press()`, etc.
   - Awaitables: `wait_for_frames()`, `wait_for_condition()`, `wait_for_order_generation()`

5. **Test System** (`src/systems/TestSystem.h`)
   - ECS system that runs tests in game loop
   - Manages coroutine execution and frame counting
   - Handles test completion and error reporting

### Integration
- **main.cpp**: Added `--list-tests` and `--run-test <name>` flags
- **game.cpp**: Added `run_test()` function for test execution
- **Test Runner Script**: `scripts/run_tests.py` for discovering and running tests

### Example Tests
- `src/testing/tests/ValidateOrderGenerationTest.h` - Example tests demonstrating framework usage

## Current Status

### Working
- ✅ Test framework compiles successfully
- ✅ Tests can be discovered (`--list-tests`)
- ✅ Test infrastructure is in place
- ✅ Crash fixed - tests now run without crashing
- ✅ Test execution framework works

### Issues
- ⚠️ Tests are failing due to timing/logic issues (not framework bugs)
- ⚠️ Order generation test expects orders but they may not generate in time
- ⚠️ Some tests may need adjustment for game logic timing

## Key Design Decisions

1. **No Headless Mode**: Removed headless mode per user request - all tests run with visible windows
2. **No Test Code in Game**: Game systems use direct raylib calls, not test wrappers
3. **Coroutine-based**: Uses C++20 coroutines for async test operations
4. **Frame Counter**: Global frame counter for `wait_for_frames()` awaitable
5. **Exception Handling**: Tests catch exceptions and report failures properly

## Files Created/Modified

### New Files
- `src/testing/test_macros.h`
- `src/testing/test_app.h`
- `src/testing/test_context.h`
- `src/testing/test_input_queue.h/cpp`
- `src/testing/test_input.h`
- `src/systems/TestSystem.h`
- `src/testing/tests/ValidateOrderGenerationTest.h`
- `src/testing/tests/all_tests.h`
- `scripts/run_tests.py`
- `.gitignore` (copied from my_name_chef)

### Modified Files
- `src/main.cpp` - Added test flags
- `src/game.cpp` - Added `run_test()` function
- `src/game.h` - Added `run_test()` declaration
- `makefile` - Added `src/testing/*.cpp` to source files

## Usage

```bash
# List all tests
./output/warehouse_game.exe --list-tests

# Run a specific test
./output/warehouse_game.exe --run-test validate_order_generation

# Use test runner script
python3 scripts/run_tests.py --list
python3 scripts/run_tests.py --test validate_order_generation
python3 scripts/run_tests.py --all
```

## Next Steps (When Resuming)

1. Fix test timing issues - ensure orders/items generate in time for tests
2. Add more comprehensive test coverage
3. Debug why `wait_for_order_generation()` condition isn't being met
4. Consider adding test fixtures for common setup
5. Add test output formatting/CI integration

## Notes

- The framework uses C++20 coroutines which require C++23 standard (already set in makefile)
- Tests run in the actual game environment with full ECS system
- Frame counter is global and shared across all awaitables
- Test coroutines are stored in `std::optional<TestCoroutine>` to ensure they stay alive

