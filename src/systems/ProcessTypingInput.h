#pragma once

#include "../components.h"
#include "../testing/test_input.h"
#include <afterhours/ah.h>

struct ProcessTypingInput : afterhours::System<TypingBuffer> {
  bool should_run(float) override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Warehouse;
  }
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Warehouse;
  }

  void for_each_with(afterhours::Entity &entity, TypingBuffer &buffer,
                     float dt) override {
    (void)entity;
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    int key = test_input::get_char_pressed();

    if (key > 0) {
      char c = static_cast<char>(key);
      if (c >= '0' && c <= '9') {
        return;
      }

      // Don't add boxing action keys to buffer when on Boxing screen
      if (active_view.current_view == ViewState::Boxing) {
        if (c == 'B' || c == 'P' || c == 'F' || c == 'T' || c == 'S' ||
            c == 'b' || c == 'p' || c == 'f' || c == 't' || c == 's') {
          return;
        }
      }

      buffer.buffer += c;
      buffer.last_input_time = 0.0f;
    }

    if (test_input::is_key_pressed(raylib::KEY_BACKSPACE) &&
        !buffer.buffer.empty()) {
      buffer.buffer.pop_back();
    }

    if (test_input::is_key_pressed(raylib::KEY_ENTER)) {
      buffer.buffer.clear();
    }

    buffer.last_input_time += dt;

    if (buffer.last_input_time > 5.0f) {
      buffer.buffer.clear();
    }
  }
};
