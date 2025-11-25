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
      return;
    }

    std::string lower_buffer = to_lower(buffer.buffer);

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    if (!selected_order.order_id.has_value()) {
      return;
    }

    for (afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      Order &order = order_entity.get<Order>();

      for (ItemType item_type : order.items) {
        std::string item_name = item_type_to_string(item_type);
        std::string lower_item = to_lower(item_name);

        if (lower_buffer == lower_item) {
          int selected_count = 0;
          for (ItemType selected : order.selected_items) {
            if (selected == item_type) {
              selected_count++;
            }
          }

          int needed_count = 0;
          for (ItemType needed : order.items) {
            if (needed == item_type) {
              needed_count++;
            }
          }

          if (selected_count < needed_count) {
            order.selected_items.push_back(item_type);
            buffer.buffer.clear();
            return;
          }
        }
      }
      break;
    }
  }
};
