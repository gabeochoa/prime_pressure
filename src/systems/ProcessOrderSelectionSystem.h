#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessOrderSelectionSystem : afterhours::System<Order> {
  bool should_run(float) const override {
    const ActiveView &active_view = get_singleton_as<ActiveView>();
    return active_view.current_view != ViewState::Cutscene;
  }

  void set_selected_and_active_order(
      std::optional<afterhours::EntityID> order_id) const {
    SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
    ActiveOrder &active_order = get_singleton_as<ActiveOrder>();
    if (order_id.has_value()) {
      selected_order.order_id.set_order_id(order_id.value());
      active_order.order_id.set_order_id(order_id.value());
    } else {
      // For resetting both to empty, access the underlying optional directly
      selected_order.order_id.order_id = order_id;
      active_order.order_id.order_id = order_id;
    }
  }

  void reset_if_matching_order(SelectedOrder &selected_order,
                               ActiveOrder &active_order,
                               afterhours::EntityID order_id) const {
    selected_order.order_id.reset_if_matching_order(order_id);
    active_order.order_id.reset_if_matching_order(order_id);
  }

  void for_each_with(afterhours::Entity &order_entity, Order &order,
                     float) override {
    SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
    ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

    afterhours::EntityID order_id = order_entity.id;

    if (!order_entity.hasTag(GameTag::IsInProgressOrder)) {
      reset_if_matching_order(selected_order, active_order, order_id);
      return;
    }

    int order_index = -1;
    for (const OrderSlot &slot : afterhours::EntityQuery()
                                     .whereID(order_entity.id)
                                     .gen_as<OrderSlot>()) {
      order_index = slot.index;
      break;
    }

    if (selected_order.order_id.is_matching_order(order_id)) {
      if (game_input::IsKeyPressed(raylib::KEY_ESCAPE)) {
        reset_if_matching_order(selected_order, active_order, order_id);
        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
        return;
      }
    }

    for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
      if (!game_input::IsKeyPressed(key)) {
        continue;
      }

      int pressed_index = key - raylib::KEY_ONE;
      if (pressed_index != order_index) {
        continue;
      }

      if (order.is_shipped() && !order.is_fully_complete()) {
        if (order.get_ready_stamp_progress() >= 3) {
          // Mark order complete by clearing its slot
          order_entity.removeComponent<OrderSlot>();
          reset_if_matching_order(selected_order, active_order, order_id);
          return;
        }

        // Shipped but not fully stamped: select and arm for stamping, do not
        // complete
        set_selected_and_active_order(order_id);
        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
        return;
      }

      if (!order.is_shipped()) {
        order.has_been_selected = true;
        if (selected_order.order_id.order_id.has_value() &&
            selected_order.order_id.order_id.value() != order_id) {
          TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
          buffer.buffer.clear();
          buffer.status = TypingStatus::Idle;
          buffer.status_time = 0.0f;
          buffer.last_input_time = 0.0f;
        }
        set_selected_and_active_order(order_id);
      }
      return;
    }
  }
};
