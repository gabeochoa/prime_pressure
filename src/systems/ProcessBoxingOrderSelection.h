#pragma once

#include "../components.h"
#include "../input_wrapper.h"
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
    // Double-check that we should run (safety check)
    afterhours::Entity &boxing_progress_entity_check =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    BoxingProgress &boxing_progress_check =
        boxing_progress_entity_check.get<BoxingProgress>();
    if (boxing_progress_check.order_id.has_value()) {
      // Order already selected, don't run
      return;
    }

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    afterhours::EntityID ready_order_id = -1;
    for (afterhours::EntityID order_id : queue.active_orders) {
      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        if (all_items_selected(order) && !order.is_shipped) {
          ready_order_id = order_id;
          break;
        }
        break;
      }
      if (ready_order_id != -1) {
        break;
      }
    }

    if (ready_order_id == -1) {
      return;
    }

    for (Order &order : afterhours::EntityQuery()
                            .whereID(ready_order_id)
                            .whereHasComponent<Order>()
                            .gen_as<Order>()) {
      afterhours::Entity &boxing_progress_entity =
          afterhours::EntityHelper::get_singleton<BoxingProgress>();
      BoxingProgress &boxing_progress =
          boxing_progress_entity.get<BoxingProgress>();

      // Clean up existing boxing items safely
      for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
        auto opt_entity = afterhours::EntityHelper::getEntityForID(item_id);
        if (opt_entity.has_value()) {
          opt_entity.value()->cleanup = true;
        }
      }
      boxing_progress.boxing_items.clear();

      // Create new boxing items
      for (ItemType item_type : order.selected_items) {
        afterhours::Entity &boxing_item_entity =
            afterhours::EntityHelper::createEntity();
        BoxingItemStatus &boxing_item =
            boxing_item_entity.addComponent<BoxingItemStatus>();
        boxing_item.type = item_type;
        boxing_item.is_placed = false;
        boxing_progress.boxing_items.push_back(boxing_item_entity.id);
      }

      order.is_complete = true;
      boxing_progress.order_id = ready_order_id;
      boxing_progress.state = BoxingState::None;
      boxing_progress.items_placed = 0;
      break;
    }
  }
};
