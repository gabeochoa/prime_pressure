#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessOrderTabbingSystem : afterhours::System<> {
private:
  std::vector<afterhours::EntityID> collect_selectable_orders() const {
    std::vector<std::pair<afterhours::EntityID, int>> orders_with_slots;
    for (const afterhours::Entity &entity :
         afterhours::EntityQuery()
             .whereHasComponent<OrderSlot>()
             .whereHasComponent<Order>()
             .whereLambda([](const afterhours::Entity &e) {
               const OrderSlot &slot = e.get<OrderSlot>();
               return slot.index >= 0 && !e.get<Order>().is_shipped();
             })
             .gen()) {
      const OrderSlot &slot = entity.get<OrderSlot>();
      orders_with_slots.emplace_back(entity.id, slot.index);
    }

    // Sort by slot index
    std::sort(orders_with_slots.begin(), orders_with_slots.end(),
              [](const auto &a, const auto &b) { return a.second < b.second; });

    // Extract just the IDs
    std::vector<afterhours::EntityID> result;
    result.reserve(orders_with_slots.size());
    for (const auto &pair : orders_with_slots) {
      result.push_back(pair.first);
    }
    return result;
  }

  std::optional<afterhours::EntityID>
  get_seed_order_id(const ActiveOrder &active_order) const {

    if (active_order.order_id.order_id.has_value()) {
      return active_order.order_id.order_id.value();
    }

    const SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
    if (selected_order.order_id.order_id.has_value()) {
      return selected_order.order_id.order_id.value();
    }
    return std::nullopt;
  }

  size_t find_current_index(
      const std::vector<afterhours::EntityID> &selected_order_ids,
      const std::optional<afterhours::EntityID> &seed_id) const {
    if (!seed_id.has_value()) {
      return 0;
    }

    for (size_t i = 0; i < selected_order_ids.size(); ++i) {
      if (selected_order_ids[i] == seed_id.value()) {
        return i;
      }
    }
    return 0;
  }

  size_t calculate_next_index(
      size_t current_index,
      const std::vector<afterhours::EntityID> &selected_order_ids,
      const std::optional<afterhours::EntityID> &seed_id) const {
    bool shift_pressed =
        seed_id.has_value() && (game_input::IsKeyDown(raylib::KEY_LEFT_SHIFT) ||
                                game_input::IsKeyDown(raylib::KEY_RIGHT_SHIFT));

    if (!seed_id.has_value()) {
      return current_index;
    }

    if (shift_pressed) {
      return (current_index + selected_order_ids.size() - 1) %
             selected_order_ids.size();
    }

    return (current_index + 1) % selected_order_ids.size();
  }

  bool update_selection(afterhours::EntityID chosen_id,
                        ActiveOrder &active_order) const {
    bool selection_changed =
        !active_order.order_id.order_id.has_value() ||
        active_order.order_id.order_id.value() != chosen_id;
    active_order.order_id.set_order_id(chosen_id);

    SelectedOrder &selected_order =
        const_cast<SelectedOrder &>(get_singleton_as<SelectedOrder>());
    if (!selected_order.order_id.order_id.has_value() ||
        selected_order.order_id.order_id.value() != chosen_id) {
      selected_order.order_id.set_order_id(chosen_id);
      selection_changed = true;
    }

    for (Order &order : afterhours::EntityQuery()
                            .whereID(chosen_id)
                            .whereHasComponent<Order>()
                            .gen_as<Order>()) {
      order.has_been_selected = true;
      break;
    }

    return selection_changed;
  }

  void handle_selection_change(bool selection_changed) const {
    if (!selection_changed) {
      return;
    }
    TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
    buffer.buffer.clear();
    buffer.status = TypingStatus::Idle;
    buffer.status_time = 0.0f;
    buffer.last_input_time = 0.0f;
  }

public:
  bool should_run(float) const override {
    const ActiveView &active_view = get_singleton_as<ActiveView>();
    return active_view.current_view != ViewState::Cutscene;
  }

  void once(float) override {
    if (!game_input::IsKeyPressed(raylib::KEY_TAB)) {
      return;
    }

    ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

    // Collect in-progress, unshipped orders for cycling
    std::vector<afterhours::EntityID> selected_order_ids =
        collect_selectable_orders();

    if (selected_order_ids.empty()) {
      active_order.order_id.order_id.reset();
      return;
    }

    // Find current active order index and cycle to next/previous
    std::optional<afterhours::EntityID> seed_id =
        get_seed_order_id(active_order);
    size_t current_index = find_current_index(selected_order_ids, seed_id);
    size_t next_index =
        calculate_next_index(current_index, selected_order_ids, seed_id);

    afterhours::EntityID chosen_id = selected_order_ids[next_index];

    // Update selection and handle changes
    bool selection_changed = update_selection(chosen_id, active_order);
    handle_selection_change(selection_changed);
  }
};