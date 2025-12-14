#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>

struct ManageConveyorItemsSystem
    : afterhours::System<ConveyorItem,
                         afterhours::tags::All<GameTag::IsOnConveyor>> {
  void for_each_with(afterhours::Entity &entity, ConveyorItem &item,
                     float dt) override {
    if (!item.is_moving) {
      return;
    }

    item.x_position += item.speed * dt;

    constexpr float READY_THRESHOLD_PCT = 0.5f; // middle of the screen
    if (item.x_position >= READY_THRESHOLD_PCT) {
      auto order_opt = afterhours::EntityQuery()
                           .whereID(item.order_id)
                           .whereHasComponent<Order>()
                           .gen_first();

      if (order_opt.has_value()) {
        afterhours::Entity &order_entity = order_opt.asE();
        Order &order = order_entity.get<Order>();
        order.ready_items.push_back(item.type);

        if (order_entity.has<OrderReceivedCounts>()) {
          OrderReceivedCounts &received =
              order_entity.get<OrderReceivedCounts>();
          received.counts[item.type]++;
        }

        entity.disableTag(GameTag::IsOnConveyor);
      }
    }
  }
};
