#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <set>
#include <vector>

struct SpawnConveyorItems : afterhours::System<> {
  void once(float) override {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    if (!selected_order.order_id.has_value()) {
      return;
    }

    // Check if there are any conveyor items for this order currently
    bool has_conveyor_items = false;
    for (const afterhours::Entity &conveyor_entity :
         afterhours::EntityQuery()
             .whereHasComponent<ConveyorItem>()
             .whereNotMarkedForCleanup()
             .gen()) {
      const ConveyorItem &existing_item = conveyor_entity.get<ConveyorItem>();
      if (existing_item.order_id == selected_order.order_id.value()) {
        has_conveyor_items = true;
        break;
      }
    }

    // Get the order to check if items have been typed
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();

      // If conveyor items already exist OR items have been typed, don't spawn
      // (items have been typed means they were spawned and shouldn't respawn)
      if (has_conveyor_items || !order.selected_items.empty()) {
        return;
      }

      // Spawn all items for this order
      std::map<ItemType, int> item_counts = count_items(order.items);
      int vertical_index = 0;

      for (const auto &[item_type, count] : item_counts) {
        for (int i = 0; i < count; ++i) {
          afterhours::Entity &conveyor_item_entity =
              afterhours::EntityHelper::createEntity();
          ConveyorItem &conveyor_item =
              conveyor_item_entity.addComponent<ConveyorItem>();
          conveyor_item.type = item_type;
          conveyor_item.x_position = ui_constants::CONVEYOR_START_X_PCT;
          conveyor_item.speed = ui_constants::CONVEYOR_SPEED;
          conveyor_item.order_id = selected_order.order_id.value();
          conveyor_item.vertical_index = vertical_index;
          conveyor_item.is_moving = false;
          vertical_index++;
        }
      }
      break;
    }
  }
};
