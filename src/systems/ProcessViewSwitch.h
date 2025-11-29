#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessViewSwitch : afterhours::System<> {
  void once(float) override {
    afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    ActiveView &active_view = view_entity.get<ActiveView>();

    bool view_changed = false;

    if (game_input::IsKeyPressed(raylib::KEY_UP)) {
      active_view.current_view = ViewState::Computer;
      view_changed = true;
    } else if (game_input::IsKeyPressed(raylib::KEY_DOWN)) {
      active_view.current_view = ViewState::Boxing;
      view_changed = true;
    } else if (game_input::IsKeyPressed(raylib::KEY_LEFT)) {
      active_view.current_view = ViewState::Warehouse;
      view_changed = true;
    } else if (game_input::IsKeyPressed(raylib::KEY_RIGHT)) {
      active_view.current_view = ViewState::Boxing;
      view_changed = true;
    } else if (game_input::IsKeyPressed(raylib::KEY_TAB)) {
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
      view_changed = true;
    }

    if (view_changed) {
      afterhours::Entity &buffer_entity =
          afterhours::EntityHelper::get_singleton<TypingBuffer>();
      TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
      buffer.buffer.clear();
      buffer.has_error = false;
      buffer.last_input_time = 0.0f;
    }
  }
};
