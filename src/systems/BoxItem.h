#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct BoxItem
    : afterhours::System<Item, afterhours::tags::All<GameTag::IsGrabbed>> {
  void for_each_with(afterhours::Entity &item_entity, Item &item,
                     float) override {
    if (!game_input::IsKeyPressed(raylib::KEY_SPACE)) {
      return;
    }

    afterhours::Entity *box_entity_ptr = nullptr;
    for (afterhours::Entity &box_entity :
         afterhours::EntityQuery().whereHasComponent<Box>().gen()) {
      if (!box_entity.hasTag(GameTag::IsBox)) {
        continue;
      }
      box_entity_ptr = &box_entity;
      break;
    }

    if (!box_entity_ptr) {
      return;
    }

    afterhours::Entity &box_entity = *box_entity_ptr;
    Box &box = box_entity.get<Box>();

    if (static_cast<int>(box.items.size()) >= box.capacity) {
      return;
    }

    item_entity.disableTag(GameTag::IsGrabbed);
    item_entity.enableTag(GameTag::IsBoxed);
    box.items.push_back(item_entity.id);

    afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    OrderQueue &queue = queue_entity.get<OrderQueue>();

    for (afterhours::EntityID order_id : queue.in_progress_orders) {
      if (order_id == -1) {
        continue;
      }
      for (Order &order : afterhours::EntityQuery()
                              .whereID(order_id)
                              .whereHasComponent<Order>()
                              .gen_as<Order>()) {

        auto it = std::find(order.items.begin(), order.items.end(), item.type);
        if (it != order.items.end()) {
          order.items_completed++;
          order.items.erase(it);

          if (order.items.empty()) {
            order.is_complete = true;
          }
          break;
        }
      }
    }
  }
};
