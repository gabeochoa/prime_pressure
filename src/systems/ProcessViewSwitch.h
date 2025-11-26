#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ProcessViewSwitch : afterhours::System<> {
  void once(float) override {
    bool tab_pressed = raylib::IsKeyPressed(raylib::KEY_TAB);
    bool shift_held = raylib::IsKeyDown(raylib::KEY_LEFT_SHIFT) ||
                      raylib::IsKeyDown(raylib::KEY_RIGHT_SHIFT);

    if (!tab_pressed) {
      return;
    }

    afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    ActiveView &active_view = view_entity.get<ActiveView>();

    if (shift_held) {
      switch (active_view.current_view) {
      case ViewState::Computer:
        active_view.current_view = ViewState::Boxing;
        break;
      case ViewState::Warehouse:
        active_view.current_view = ViewState::Computer;
        break;
      case ViewState::Boxing:
        active_view.current_view = ViewState::Warehouse;
        break;
      }
    } else {
      switch (active_view.current_view) {
      case ViewState::Computer:
        active_view.current_view = ViewState::Warehouse;
        break;
      case ViewState::Warehouse:
        active_view.current_view = ViewState::Boxing;
        break;
      case ViewState::Boxing:
        active_view.current_view = ViewState::Computer;
        break;
      }
    }

    afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
    buffer.buffer.clear();
    buffer.has_error = false;
    buffer.last_input_time = 0.0f;
  }
};
