#pragma once

#include "../components.h"
#include <afterhours/ah.h>

static std::string to_lower(const std::string &str) {
  std::string result;
  result.reserve(str.size());
  for (char c : str) {
    result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return result;
}

struct MatchItemToOrder : afterhours::System<> {
  void once(float) override {
    afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
    if (buffer.buffer.empty()) {
      return;
    }

    std::string lower_buffer = to_lower(buffer.buffer);

    afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    OrderQueue &queue = queue_entity.get<OrderQueue>();

    for (afterhours::EntityID order_id : queue.active_orders) {
      for (afterhours::Entity &order_entity : afterhours::EntityQuery()
                                                  .whereID(order_id)
                                                  .whereHasComponent<Order>()
                                                  .gen()) {
        Order &order = order_entity.get<Order>();

        for (ItemType item_type : order.items) {
          std::string item_name = item_type_to_string(item_type);
          std::string lower_item = to_lower(item_name);

          if (lower_buffer == lower_item) {
            for (afterhours::Entity &item_entity :
                 afterhours::EntityQuery()
                     .whereHasComponent<Item>()
                     .whereLambda([item_type](const afterhours::Entity &e) {
                       return e.hasTag(GameTag::IsOnShelf) &&
                              e.get<Item>().type == item_type;
                     })
                     .gen()) {
              item_entity.disableTag(GameTag::IsOnShelf);
              item_entity.enableTag(GameTag::IsGrabbed);
              buffer.buffer.clear();
              return;
            }
          }
        }
        break;
      }
    }
  }
};
