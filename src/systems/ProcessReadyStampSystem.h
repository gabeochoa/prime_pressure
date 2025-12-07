#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessReadyStampSystem : afterhours::System<> {
  bool should_run(float) const override {
    return true; // Always run for debugging
  }

  void once(float) override {
    // Get active order, fallback to selected order
    const ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

    std::optional<afterhours::EntityID> target_order_id;
    if (active_order.order_id.order_id.has_value()) {
      target_order_id = active_order.order_id.order_id.value();
    } else {
      const SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
      if (selected_order.order_id.order_id.has_value()) {
        target_order_id = selected_order.order_id.order_id.value();
      }
    }

    if (!target_order_id.has_value()) {
      return;
    }

    bool pressed_r = game_input::IsKeyPressed(raylib::KEY_R);
    bool pressed_t = game_input::IsKeyPressed(raylib::KEY_T);
    bool pressed_s = game_input::IsKeyPressed(raylib::KEY_S);

    if (!pressed_r && !pressed_t && !pressed_s) {
      return;
    }

    char c = ' ';
    if (pressed_r) {
      c = 'r';
    } else if (pressed_t) {
      c = 't';
    } else if (pressed_s) {
      c = 's';
    }

    for (afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(target_order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      Order &order = order_entity.get<Order>();
      // Allow stamping on orders that aren't fully complete
      if (order.is_fully_complete()) {
        break;
      }

      const char sequence[3] = {'r', 't', 's'};
      int progress = order.get_ready_stamp_progress();
      if (progress < 3 && c == sequence[progress]) {
        order.set_ready_stamp_progress(progress + 1);
      }
      break;
    }
  }
};
