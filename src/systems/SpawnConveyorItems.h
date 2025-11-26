#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <set>
#include <vector>

struct SpawnConveyorItems : afterhours::System<> {
  void once(float) override {
    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    int global_vertical_offset = 0;

    for (afterhours::EntityID order_id : queue.active_orders) {
      if (order_id == -1) {
        continue;
      }
      bool has_conveyor_items = false;
      for (const ConveyorItem &existing_item :
           afterhours::EntityQuery()
               .whereHasComponent<ConveyorItem>()
               .whereNotMarkedForCleanup()
               .gen_as<ConveyorItem>()) {
        if (existing_item.order_id == order_id) {
          has_conveyor_items = true;
          break;
        }
      }

      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        if (has_conveyor_items || !order.selected_items.empty()) {
          int max_vertical_index = -1;
          for (const ConveyorItem &existing_item :
               afterhours::EntityQuery()
                   .whereHasComponent<ConveyorItem>()
                   .whereNotMarkedForCleanup()
                   .gen_as<ConveyorItem>()) {
            if (existing_item.order_id == order_id &&
                existing_item.vertical_index > max_vertical_index) {
              max_vertical_index = existing_item.vertical_index;
            }
          }
          if (max_vertical_index >= 0) {
            global_vertical_offset = max_vertical_index + 1;
          }
          break;
        }

        std::map<ItemType, int> item_counts = count_items(order.items);
        int vertical_index = global_vertical_offset;

        for (const auto &[item_type, count] : item_counts) {
          for (int i = 0; i < count; ++i) {
            afterhours::Entity &conveyor_item_entity =
                afterhours::EntityHelper::createEntity();
            ConveyorItem &conveyor_item =
                conveyor_item_entity.addComponent<ConveyorItem>();
            conveyor_item.type = item_type;
            conveyor_item.x_position = ui_constants::CONVEYOR_START_X_PCT;
            conveyor_item.speed = ui_constants::CONVEYOR_SPEED;
            conveyor_item.order_id = order_id;
            conveyor_item.vertical_index = vertical_index;
            conveyor_item.is_moving = false;
            vertical_index++;
          }
        }
        global_vertical_offset = vertical_index;
        break;
      }
    }
  }
};
