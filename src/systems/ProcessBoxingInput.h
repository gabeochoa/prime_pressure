#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessBoxingInput : afterhours::System<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Boxing;
  }

  void once(float) override {
    afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    BoxingProgress &boxing_progress =
        boxing_progress_entity.get<BoxingProgress>();

    if (!boxing_progress.order_id.has_value()) {
      return;
    }

    for (afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(boxing_progress.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      Order &order = order_entity.get<Order>();

      if (boxing_progress.state == BoxingState::FoldBox &&
          game_input::IsKeyPressed(raylib::KEY_B)) {
        boxing_progress.state = BoxingState::PutItems;
        break;
      }

      if (boxing_progress.state == BoxingState::PutItems &&
          game_input::IsKeyPressed(raylib::KEY_P)) {
        boxing_progress.items_placed++;
        int total_items = static_cast<int>(order.selected_items.size());
        if (boxing_progress.items_placed >= total_items) {
          boxing_progress.state = BoxingState::Fold;
        }
        break;
      }

      if (boxing_progress.state == BoxingState::Fold &&
          game_input::IsKeyPressed(raylib::KEY_F)) {
        boxing_progress.state = BoxingState::Tape;
        break;
      }

      if (boxing_progress.state == BoxingState::Tape &&
          game_input::IsKeyPressed(raylib::KEY_T)) {
        boxing_progress.state = BoxingState::Ship;
        break;
      }

      if (boxing_progress.state == BoxingState::Ship &&
          game_input::IsKeyPressed(raylib::KEY_S)) {
        order.is_shipped = true;
        boxing_progress.order_id.reset();
        boxing_progress.state = BoxingState::None;
        boxing_progress.items_placed = 0;
        break;
      }
      break;
    }
  }
};
