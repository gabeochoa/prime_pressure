#pragma once

#include "../components.h"
#include "../eq.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>

static std::string to_lower(const std::string &str) {
  std::string result;
  result.reserve(str.size());
  for (char c : str) {
    result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return result;
}

inline bool mark_conveyor_item_as_moving(afterhours::EntityID order_id,
                                         ItemType item_type) {
  auto conveyor_opt =
      EQ().whereHasComponent<ConveyorItem>()
          .whereHasTag(GameTag::IsOnConveyor)
          .whereHasOrderID(order_id)
          .whereLambda([&](const afterhours::Entity &entity) {
            const ConveyorItem &conveyor_item = entity.get<ConveyorItem>();
            return conveyor_item.type == item_type && !conveyor_item.is_moving;
          })
          .gen_first();
  if (conveyor_opt.has_value()) {
    ConveyorItem &conveyor_item = conveyor_opt->get<ConveyorItem>();
    conveyor_item.is_moving = true;
    return true;
  }
  return false;
}

struct MatchItemToOrder
    : afterhours::System<Order,
                         afterhours::tags::All<GameTag::IsSelectedOrder>> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Warehouse;
  }

  void for_each_with(afterhours::Entity &order_entity, Order &order,
                     float) override {

    afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();

    if (buffer.buffer.empty()) {
      buffer.has_error = false;
      return;
    }

    if (buffer.buffer.length() != 1) {
      buffer.has_error = false;
      return;
    }

    char typed_key = buffer.buffer[0];

    std::map<ItemType, int> item_counts = count_items(order.items);
    std::map<ItemType, int> selected_counts = count_items(order.selected_items);

    for (const auto &[item_type, needed_count] : item_counts) {
      int selected_count = selected_counts[item_type];
      if (selected_count >= needed_count) {
        continue;
      }

      auto key_it = ITEM_KEY_MAP.find(item_type);
      if (key_it == ITEM_KEY_MAP.end()) {
        continue;
      }

      char item_key = key_it->second;
      if (typed_key == item_key) {
        order.selected_items.push_back(item_type);
        mark_conveyor_item_as_moving(order_entity.id, item_type);

        buffer.buffer.clear();
        buffer.has_error = false;
        return;
      }
    }

    buffer.has_error = true;
  }
};
