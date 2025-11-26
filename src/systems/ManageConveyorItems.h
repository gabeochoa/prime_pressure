#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>

struct ManageConveyorItems : afterhours::System<ConveyorItem> {
  void for_each_with(afterhours::Entity &entity, ConveyorItem &item,
                     float dt) override {
    if (!item.is_moving) {
      return;
    }

    item.x_position += item.speed * dt;

    if (item.x_position >= ui_constants::CONVEYOR_END_X_PCT) {
      bool added_to_ready = false;
      for (Order &order : afterhours::EntityQuery()
                              .whereID(item.order_id)
                              .whereHasComponent<Order>()
                              .gen_as<Order>()) {
        order.ready_items.push_back(item.type);
        added_to_ready = true;
        break;
      }
      if (added_to_ready) {
        entity.cleanup = true;
      }
    }
  }
};
