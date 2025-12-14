#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct BoxItemSystem
    : afterhours::System<Item, afterhours::tags::All<GameTag::IsGrabbed>> {
  void for_each_with(afterhours::Entity &item_entity, Item &item,
                     float) override {
    if (!game_input::IsKeyPressed(raylib::KEY_SPACE)) {
      return;
    }

    if (!afterhours::EntityQuery()
             .whereHasTag(GameTag::IsBox)
             .whereHasComponent<Box>()
             .whereLambda([](const afterhours::Entity &entity) {
               const Box &b = entity.get<Box>();
               return static_cast<int>(b.items.size()) < b.capacity;
             })
             .has_values()) {
      return;
    }

    Box &box = afterhours::EntityQuery()
                   .whereHasTag(GameTag::IsBox)
                   .whereHasComponent<Box>()
                   .whereLambda([](const afterhours::Entity &entity) {
                     const Box &b = entity.get<Box>();
                     return static_cast<int>(b.items.size()) < b.capacity;
                   })
                   .gen_first_as<Box>();

    item_entity.disableTag(GameTag::IsGrabbed);
    item_entity.enableTag(GameTag::IsBoxed);
    box.items.push_back(item_entity.id);

    for (Order &order : afterhours::EntityQuery()
                            .whereHasComponent<Order>()
                            .whereHasComponent<OrderSlot>()
                            .whereLambda([&](const afterhours::Entity &entity) {
                              const Order &o = entity.get<Order>();
                              return std::find(o.items.begin(), o.items.end(),
                                               item.type) != o.items.end();
                            })
                            .gen_as<Order>()) {
      auto it = std::find(order.items.begin(), order.items.end(), item.type);
      if (it != order.items.end()) {
        order.items_completed++;
        order.items.erase(it);
        // TODO: Update OrderBoxedCounts component when items are boxed
        // Consider: Add system to increment boxed_counts[item.type] for the order
        // This is needed for the state machine to properly track completion
        break;
      }
    }
  }
};
