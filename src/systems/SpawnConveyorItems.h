#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <set>
#include <vector>

struct ConveyorItemQuery : afterhours::EntityQuery<ConveyorItemQuery> {
  struct WhereHasOrderID : Modification {
    afterhours::EntityID order_id;
    explicit WhereHasOrderID(afterhours::EntityID id) : order_id(id) {}
    bool operator()(const afterhours::Entity &entity) const override {
      if (!entity.has<ConveyorItem>()) {
        return false;
      }
      const ConveyorItem &item = entity.get<ConveyorItem>();
      return item.order_id == order_id;
    }
  };

  ConveyorItemQuery &whereHasOrderID(afterhours::EntityID order_id) {
    return add_mod(new WhereHasOrderID(order_id));
  }
};

inline int get_max_vertical_index() {
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
  return max_vertical_index;
}

inline void
create_conveyor_items_for_order(afterhours::EntityID order_id,
                                const std::map<ItemType, int> &item_counts,
                                int start_vertical_index) {
  int vertical_index = start_vertical_index;
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

struct SpawnConveyorItems
    : afterhours::System<Order,
                         afterhours::tags::All<GameTag::IsInProgressOrder>> {
  void for_each_with(afterhours::Entity &order_entity, Order &order,
                     float) override {
    if (!order.selected_items.empty()) {
      return;
    }

    afterhours::EntityID order_id = order_entity.id;

    bool has_conveyor_items = ConveyorItemQuery()
                                  .whereHasComponent<ConveyorItem>()
                                  .whereHasTag(GameTag::IsOnConveyor)
                                  .whereHasOrderID(order_id)
                                  .has_values();

    if (has_conveyor_items) {
      return;
    }

    int max_vertical_index = get_max_vertical_index();
    std::map<ItemType, int> item_counts = count_items(order.items);
    create_conveyor_items_for_order(order_id, item_counts,
                                    max_vertical_index + 1);
  }
};
