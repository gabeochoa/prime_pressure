#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessOrderSelection : afterhours::System<Order> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Computer;
  }

  void for_each_with(afterhours::Entity &order_entity, Order &order,
                     float) override {
    afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();

    afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    OrderQueue &queue = queue_entity.get<OrderQueue>();

    afterhours::EntityID order_id = order_entity.id;

    if (!order_entity.hasTag(GameTag::IsInProgressOrder)) {
      if (selected_order.order_id.has_value() &&
          selected_order.order_id.value() == order_id) {
        selected_order.order_id.reset();
      }
      return;
    }

    int order_index = -1;
    for (size_t i = 0; i < queue.in_progress_orders.size(); ++i) {
      if (queue.in_progress_orders[i] == order_id) {
        order_index = static_cast<int>(i);
        break;
      }
    }

    if (selected_order.order_id.has_value() &&
        selected_order.order_id.value() == order_id) {
      if (game_input::IsKeyPressed(raylib::KEY_ESCAPE)) {
        selected_order.order_id.reset();
        afterhours::Entity &buffer_entity =
            afterhours::EntityHelper::get_singleton<TypingBuffer>();
        TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
        buffer.buffer.clear();
        buffer.has_error = false;
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

      if (order.is_shipped && !order.is_fully_complete) {
        order.is_fully_complete = true;
        if (!queue.pending_orders.empty()) {
          afterhours::EntityID new_order_id = queue.pending_orders.front();
          queue.pending_orders.erase(queue.pending_orders.begin());
          queue.in_progress_orders[order_index] = new_order_id;
        } else {
          queue.in_progress_orders[order_index] = -1;
        }
        if (selected_order.order_id.has_value() &&
            selected_order.order_id.value() == order_id) {
          selected_order.order_id.reset();
        }
        return;
      }

      if (!order.is_shipped) {
        order.has_been_selected = true;
        if (selected_order.order_id.has_value() &&
            selected_order.order_id.value() != order_id) {
          afterhours::Entity &buffer_entity =
              afterhours::EntityHelper::get_singleton<TypingBuffer>();
          TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
          buffer.buffer.clear();
          buffer.has_error = false;
        }
        selected_order.order_id = order_id;
      }
      return;
    }
  }
};
