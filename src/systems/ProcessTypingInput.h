#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ProcessTypingInput : afterhours::System<TypingBuffer> {
  void for_each_with(afterhours::Entity &entity, TypingBuffer &buffer,
                     float dt) override {
    int key = raylib::GetCharPressed();

    if (key > 0) {
      char c = static_cast<char>(key);
      if (c >= '0' && c <= '9') {
        return;
      }
      buffer.buffer += c;
      buffer.last_input_time = 0.0f;
    }

    if (raylib::IsKeyPressed(raylib::KEY_BACKSPACE) && !buffer.buffer.empty()) {
      buffer.buffer.pop_back();
    }

    buffer.last_input_time += dt;

    if (buffer.last_input_time > 5.0f) {
      buffer.buffer.clear();
    }
  }
};
