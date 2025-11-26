#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessOrderSelection : afterhours::System<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Computer;
  }

  void once(float) override {
    afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();

    afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    OrderQueue &queue = queue_entity.get<OrderQueue>();

    if (selected_order.order_id.has_value()) {
      auto it =
          std::find(queue.active_orders.begin(), queue.active_orders.end(),
                    selected_order.order_id.value());
      if (it == queue.active_orders.end()) {
        selected_order.order_id.reset();
      }
    }

    if (game_input::IsKeyPressed(raylib::KEY_ESCAPE) &&
        selected_order.order_id.has_value()) {
      selected_order.order_id.reset();
      afterhours::Entity &buffer_entity =
          afterhours::EntityHelper::get_singleton<TypingBuffer>();
      TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
      buffer.buffer.clear();
      buffer.has_error = false;
      return;
    }

    for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
      if (!game_input::IsKeyPressed(key)) {
        continue;
      }

      int order_index = key - raylib::KEY_ONE;
      if (order_index < 0 ||
          order_index >= static_cast<int>(queue.active_orders.size())) {
        break;
      }

      afterhours::EntityID order_id = queue.active_orders[order_index];
      if (order_id == -1) {
        break;
      }

      for (Order &order : afterhours::EntityQuery()
                              .whereID(order_id)
                              .whereHasComponent<Order>()
                              .gen_as<Order>()) {

        if (order.is_shipped && !order.is_fully_complete) {
          order.is_fully_complete = true;
          auto it = std::find(queue.active_orders.begin(),
                              queue.active_orders.end(), order_id);
          if (it != queue.active_orders.end()) {
            int position = static_cast<int>(it - queue.active_orders.begin());
            if (!queue.pending_orders.empty()) {
              afterhours::EntityID new_order_id = queue.pending_orders.front();
              queue.pending_orders.erase(queue.pending_orders.begin());
              queue.active_orders[position] = new_order_id;
            } else {
              queue.active_orders[position] = -1;
            }
          }
          if (selected_order.order_id.has_value() &&
              selected_order.order_id.value() == order_id) {
            selected_order.order_id.reset();
          }
          break;
        }

        if (!order.is_shipped) {
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
        break;
      }
      break;
    }
  }
};
