#pragma once

#include "../components.h"
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

struct MatchItemToOrder : afterhours::System<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Warehouse;
  }

  void once(float) override {
    afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
    if (buffer.buffer.empty()) {
      buffer.has_error = false;
      return;
    }

    std::string lower_buffer = to_lower(buffer.buffer);

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    if (!selected_order.order_id.has_value()) {
      buffer.has_error = false;
      return;
    }

    bool found_match = false;
    for (afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      Order &order = order_entity.get<Order>();

      std::map<ItemType, int> item_counts = count_items(order.items);
      std::map<ItemType, int> selected_counts =
          count_items(order.selected_items);

      for (const auto &[item_type, needed_count] : item_counts) {
        int selected_count = selected_counts[item_type];
        if (selected_count >= needed_count) {
          continue;
        }

        std::string item_name = item_type_to_string(item_type);
        std::string lower_item = to_lower(item_name);

        if (lower_buffer == lower_item) {
          order.selected_items.push_back(item_type);

          auto conveyor_opt =
              afterhours::EntityQuery()
                  .whereHasComponent<ConveyorItem>()
                  .whereLambda([&](const afterhours::Entity &entity) {
                    const ConveyorItem &conveyor_item =
                        entity.get<ConveyorItem>();
                    return conveyor_item.type == item_type &&
                           conveyor_item.order_id ==
                               selected_order.order_id.value() &&
                           !conveyor_item.is_moving;
                  })
                  .gen_first();
          if (conveyor_opt.has_value()) {
            ConveyorItem &conveyor_item = conveyor_opt->get<ConveyorItem>();
            conveyor_item.is_moving = true;
          }

          buffer.buffer.clear();
          buffer.has_error = false;
          return;
        }

        if (lower_item.find(lower_buffer) == 0) {
          found_match = true;
        }
      }
      break;
    }

    buffer.has_error = !found_match;
  }
};
