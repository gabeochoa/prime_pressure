#pragma once

#include "../components.h"
#include "../input_wrapper.h"
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

    int key = game_input::GetCharPressed();

    if (key > 0) {
      char c = static_cast<char>(key);
      if (c >= '0' && c <= '9') {
        return;
      }

      if (active_view.current_view == ViewState::Boxing) {
        if (c == 'B' || c == 'P' || c == 'F' || c == 'T' || c == 'S' ||
            c == 'b' || c == 'p' || c == 'f' || c == 't' || c == 's') {
          return;
        }
      }

      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        buffer.buffer =
            static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        buffer.last_input_time = 0.0f;
        buffer.has_error = false;
      }
    }

    buffer.last_input_time += dt;

    if (buffer.last_input_time > 2.0f) {
      buffer.buffer.clear();
      buffer.has_error = false;
    }
  }
};
