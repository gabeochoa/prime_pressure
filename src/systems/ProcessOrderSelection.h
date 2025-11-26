#pragma once

#include "../components.h"
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

    if (raylib::IsKeyPressed(raylib::KEY_ESCAPE) &&
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
      if (!raylib::IsKeyPressed(key)) {
        continue;
      }

      int order_index = key - raylib::KEY_ONE;
      if (order_index < 0 ||
          order_index >= static_cast<int>(queue.active_orders.size())) {
        break;
      }

      afterhours::EntityID order_id = queue.active_orders[order_index];

      for (afterhours::Entity &order_entity : afterhours::EntityQuery()
                                                  .whereID(order_id)
                                                  .whereHasComponent<Order>()
                                                  .gen()) {
        Order &order = order_entity.get<Order>();

        if (order.is_shipped && !order.is_fully_complete) {
          order.is_fully_complete = true;
          auto it = std::find(queue.active_orders.begin(),
                              queue.active_orders.end(), order_id);
          if (it != queue.active_orders.end()) {
            queue.active_orders.erase(it);
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
