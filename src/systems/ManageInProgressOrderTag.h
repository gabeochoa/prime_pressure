#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ManageInProgressOrderTag : afterhours::System<Order> {
  void for_each_with(afterhours::Entity &order_entity, Order &,
                     float) override {
    afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    OrderQueue &queue = queue_entity.get<OrderQueue>();

    afterhours::EntityID order_id = order_entity.id;
    bool is_in_progress_orders = false;

    for (afterhours::EntityID in_progress_id : queue.in_progress_orders) {
      if (in_progress_id == order_id) {
        is_in_progress_orders = true;
        break;
      }
    }

    bool has_tag = order_entity.hasTag(GameTag::IsInProgressOrder);

    if (is_in_progress_orders && !has_tag) {
      order_entity.enableTag(GameTag::IsInProgressOrder);
    } else if (!is_in_progress_orders && has_tag) {
      order_entity.disableTag(GameTag::IsInProgressOrder);
    }
  }
};
