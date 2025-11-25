#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ProcessBoxingOrderSelection : afterhours::System<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    if (active_view.current_view != ViewState::Boxing) {
      return false;
    }

    const afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    const BoxingProgress &boxing_progress =
        boxing_progress_entity.get<BoxingProgress>();
    return !boxing_progress.order_id.has_value();
  }

  void once(float) override {
    for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
      if (!raylib::IsKeyPressed(key)) {
        continue;
      }

      int order_index = key - raylib::KEY_ONE;
      const afterhours::Entity &queue_entity =
          afterhours::EntityHelper::get_singleton<OrderQueue>();
      const OrderQueue &queue = queue_entity.get<OrderQueue>();

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

        if (!all_items_selected(order) || order.is_shipped) {
          break;
        }

        afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::get_singleton<BoxingProgress>();
        BoxingProgress &boxing_progress =
            boxing_progress_entity.get<BoxingProgress>();

        order.is_complete = true;
        boxing_progress.order_id = order_id;
        boxing_progress.state = BoxingState::FoldBox;
        boxing_progress.items_placed = 0;
        break;
      }
      break;
    }
  }
};
