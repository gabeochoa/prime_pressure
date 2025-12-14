#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include <afterhours/ah.h>

struct ProcessReadyStampSystem : afterhours::System<Order, OrderWorkflow> {
  bool should_run(float) const override {
    return true;
  }

  void for_each_with(afterhours::Entity &order_entity, Order &order, OrderWorkflow &workflow, float) override {
    // Get active order, fallback to selected order
    const ActiveOrder &active_order = get_singleton_as<ActiveOrder>();
    const SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();

    bool is_active_order = active_order.order_id.is_matching_order(order_entity.id);
    bool is_selected_order = selected_order.order_id.is_matching_order(order_entity.id);

    // Only process input for orders that are either actively being worked on
    // or are currently selected in the UI - skip all others
    if (!is_active_order && !is_selected_order) {
      return;
    }

    // Only process if we're in a shipped stamping state
    if (kind_of(workflow.state) != OrderStateKind::Input ||
        macro_state_of(workflow.state) != OrderMacroState::Shipped) {
      return;
    }

    bool pressed_r = game_input::IsKeyPressed(raylib::KEY_R);
    bool pressed_t = game_input::IsKeyPressed(raylib::KEY_T);
    bool pressed_s = game_input::IsKeyPressed(raylib::KEY_S);

    if (!pressed_r && !pressed_t && !pressed_s) {
      return;
    }

    // Determine expected key based on current state
    char expected_key = ' ';
    OrderState next_state = workflow.state;

    switch (workflow.state) {
      case OrderState::Shipped_Stamp0:
        if (pressed_r) {
          expected_key = 'r';
          next_state = OrderState::Shipped_Stamp1;
        }
        break;
      case OrderState::Shipped_Stamp1:
        if (pressed_t) {
          expected_key = 't';
          next_state = OrderState::Shipped_Stamp2;
        }
        break;
      case OrderState::Shipped_Stamp2:
        if (pressed_s) {
          expected_key = 's';
          next_state = OrderState::Shipped_Stamp3;
        }
        break;
      case OrderState::Shipped_Stamp3:
        // READY TO SHIP confirmed - advance to closeout delay
        if (pressed_s) {  // Allow any key? Or specific sequence?
          expected_key = 's'; // For now, accept S
          next_state = OrderState::Complete_CloseoutDelay;
        }
        break;
      default:
        return;
    }

    if (expected_key != ' ') {
      workflow.state = next_state;
      workflow.time_in_state = 0.0f;

      log_info("Order {} shipped stamp advanced to state {}",
               static_cast<unsigned long long>(order_entity.id),
               static_cast<int>(workflow.state));
    }
  }
};
