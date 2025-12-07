#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct UpdateTimelineStateSystem : afterhours::System<Order> {
private:
  bool is_conveyor_full(afterhours::EntityID order_id, const Order &order) const {
    std::map<ItemType, int> item_counts = count_items(order.items);
    std::map<ItemType, int> ready_counts = count_items(order.ready_items);

    std::map<ItemType, int> conveyor_counts;
    for (const ConveyorItem &ci : afterhours::EntityQuery()
                                      .whereHasComponent<ConveyorItem>()
                                      .gen_as<ConveyorItem>()) {
      if (ci.order_id == order_id) {
        conveyor_counts[ci.type]++;
      }
    }

    for (const auto &[item_type, count] : item_counts) {
      int on_belt = conveyor_counts[item_type] + ready_counts[item_type];
      if (on_belt < count) {
        return false;
      }
    }
    return true;
  }

  bool all_items_ready(const Order &order) const {
    std::map<ItemType, int> item_counts = count_items(order.items);
    std::map<ItemType, int> ready_counts = count_items(order.ready_items);

    for (const auto &[item_type, count] : item_counts) {
      if (ready_counts[item_type] < count) {
        return false;
      }
    }
    return true;
  }

  bool conveyor_empty(afterhours::EntityID order_id) const {
    for (const ConveyorItem &ci : afterhours::EntityQuery()
                                      .whereHasComponent<ConveyorItem>()
                                      .gen_as<ConveyorItem>()) {
      if (ci.order_id == order_id) {
        return false;
      }
    }
    return true;
  }

  bool should_flash(afterhours::EntityID order_id) const {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();
    return selected_order.order_id.order_id.has_value() &&
           selected_order.order_id.order_id.value() == order_id;
  }

  bool should_advance_to_next_state(const Order &order, afterhours::EntityID order_id) const {
    switch (order.timeline.state) {
    case TimelineStageState::ConveyorPending:
      return !order.items.empty();
    case TimelineStageState::ConveyorActive:
    case TimelineStageState::ConveyorActiveFlash:
      return is_conveyor_full(order_id, order);
    case TimelineStageState::ConveyorDone:
      return all_items_ready(order) && conveyor_empty(order_id);
    case TimelineStageState::BoxingActive:
      return order.is_shipped();
    case TimelineStageState::ReadyActive:
      return order.get_ready_stamp_progress() >= 1;
    case TimelineStageState::ReadyStamp0:
      return order.get_ready_stamp_progress() >= 1;
    case TimelineStageState::ReadyStamp1:
      return order.get_ready_stamp_progress() >= 2;
    case TimelineStageState::ReadyStamp2:
      return order.get_ready_stamp_progress() >= 3;
    case TimelineStageState::ReadyStamp3:
      return order.is_fully_complete();
    case TimelineStageState::BoxingPending:
    case TimelineStageState::BoxingDone:
    case TimelineStageState::ReadyPending:
    case TimelineStageState::ReadyDone:
    case TimelineStageState::ShipPending:
    case TimelineStageState::ShipDone:
    default:
      return false;
    }
  }

  TimelineStageState get_next_state(TimelineStageState current) const {
    int next_value = static_cast<int>(current) + 1;
    // Cap at ShipDone
    if (next_value > static_cast<int>(TimelineStageState::ShipDone)) {
      return TimelineStageState::ShipDone;
    }
    return static_cast<TimelineStageState>(next_value);
  }

public:
  void for_each_with(afterhours::Entity &order_entity, Order &order,
                     float) override {
    if (order.items.empty()) {
      return; // No items, stay in initial state
    }

    // Check if we should advance to the next state
    if (should_advance_to_next_state(order, order_entity.id)) {
      order.timeline.state = get_next_state(order.timeline.state);
    }

    // Handle flashing for conveyor active state
    if (order.timeline.state == TimelineStageState::ConveyorActive && should_flash(order_entity.id)) {
      order.timeline.state = TimelineStageState::ConveyorActiveFlash;
    } else if (order.timeline.state == TimelineStageState::ConveyorActiveFlash && !should_flash(order_entity.id)) {
      order.timeline.state = TimelineStageState::ConveyorActive;
    }
  }
};

