#pragma once

#include <afterhours/ah.h>

#include "../testing/test_input.h"
#include "../testing/test_app.h"

// Ensures test input consumption flags + frame counter advance at the start of
// a frame (before any other systems read input).
struct TestFrameBeginSystem : afterhours::System<> {
  void once(float) override {
    if (!test_input::test_mode) {
      return;
    }
    test_input::reset_frame();
    test_app::frame_counter++;
  }
};

