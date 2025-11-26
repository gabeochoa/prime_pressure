#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <set>
#include <vector>

struct SpawnConveyorItems
    : afterhours::System<Order,
                         afterhours::tags::All<GameTag::IsInProgressOrder>> {
  void for_each_with(afterhours::Entity &order_entity, Order &order,
                     float) override {
    afterhours::EntityID order_id = order_entity.id;

    bool has_conveyor_items = false;
    for (const ConveyorItem &existing_item :
         afterhours::EntityQuery()
             .whereHasComponent<ConveyorItem>()
             .whereHasTag(GameTag::IsOnConveyor)
             .gen_as<ConveyorItem>()) {
      // TODO add whereHasOrderID()
      if (existing_item.order_id == order_id) {
        has_conveyor_items = true;
        break;
      }
    }

    if (has_conveyor_items || !order.selected_items.empty()) {
      return;
    }

    // TODO if this is for rendering it should be in the rendering system
    int max_vertical_index = -1;
    for (const ConveyorItem &existing_item :
         afterhours::EntityQuery()
             .whereHasComponent<ConveyorItem>()
             .whereHasTag(GameTag::IsOnConveyor)
             .gen_as<ConveyorItem>()) {
      if (existing_item.vertical_index > max_vertical_index) {
        max_vertical_index = existing_item.vertical_index;
      }
    }

    int vertical_index = max_vertical_index + 1;
    std::map<ItemType, int> item_counts = count_items(order.items);

    // TODO make a  helper to ahandle all of this
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
        conveyor_item_entity.enableTag(GameTag::IsOnConveyor);
        vertical_index++;
      }
    }
  }
};
