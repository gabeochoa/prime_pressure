#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ManageInProgressOrderTagSystem : afterhours::System<Order> {
  void for_each_with(afterhours::Entity &order_entity, Order &,
                     float) override {
    bool has_valid_slot =
        afterhours::EntityQuery()
            .whereID(order_entity.id)
            .whereHasComponent<OrderSlot>()
            .whereLambda([](const afterhours::Entity &entity) {
              const OrderSlot &slot = entity.get<OrderSlot>();
              return slot.index >= 0;
            })
            .has_values();

    order_entity.setTag(GameTag::IsInProgressOrder, has_valid_slot);
  }
};
