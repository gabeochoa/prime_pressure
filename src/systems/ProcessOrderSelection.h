#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ProcessOrderSelection : afterhours::System<> {
  void once(float) override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Computer) {
      return;
    }

    afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();

    if (selected_order.order_id.has_value()) {
      return;
    }

    for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
      if (raylib::IsKeyPressed(key)) {
        int order_index = key - raylib::KEY_ONE;

        const afterhours::Entity &queue_entity =
            afterhours::EntityHelper::get_singleton<OrderQueue>();
        const OrderQueue &queue = queue_entity.get<OrderQueue>();

        if (order_index >= 0 &&
            order_index < static_cast<int>(queue.active_orders.size())) {
          selected_order.order_id = queue.active_orders[order_index];
        }
        break;
      }
    }
  }
};
