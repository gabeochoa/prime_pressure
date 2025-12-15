#pragma once

#include "rl.h"
#include "testing/test_input.h"

namespace game_input {
inline bool IsKeyPressed(int key) {
    if (test_input::test_mode) {
        // Deterministic test mode: never fall back to real input.
        if (test_input::input_queue.empty() ||
            test_input::key_consumed_this_frame) {
            return false;
        }

        if (!test_input::input_queue.front().is_char &&
            test_input::input_queue.front().key == key) {
            test_input::input_queue.pop();
            test_input::key_consumed_this_frame = true;
            return true;
        }

        return false;
    }

    return raylib::IsKeyPressed(key);
}

inline bool IsKeyDown(int key) { return raylib::IsKeyDown(key); }

inline int GetCharPressed() {
    if (test_input::test_mode) {
        // Deterministic test mode: never fall back to real input.
        if (test_input::input_queue.empty() ||
            test_input::char_consumed_this_frame) {
            return 0;
        }
        if (test_input::input_queue.front().is_char) {
            char c = test_input::input_queue.front().char_value;
            test_input::input_queue.pop();
            test_input::char_consumed_this_frame = true;
            return static_cast<int>(c);
        }
        return 0;
    }

    return raylib::GetCharPressed();
}
}  // namespace game_input
