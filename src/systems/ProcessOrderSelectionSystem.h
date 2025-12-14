#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include <afterhours/ah.h>

struct ProcessOrderSelectionSystem : afterhours::System<Order, OrderWorkflow> {
  bool should_run(float) const override {
    return true;
  }

  void set_selected_and_active_order(
      std::optional<afterhours::EntityID> order_id) const {
    SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
    ActiveOrder &active_order = get_singleton_as<ActiveOrder>();
    if (order_id.has_value()) {
      selected_order.order_id.set_order_id(order_id.value());
      active_order.order_id.set_order_id(order_id.value());
    } else {
      // For resetting both to empty, access the underlying optional directly
      selected_order.order_id.order_id = order_id;
      active_order.order_id.order_id = order_id;
    }
  }

  void reset_if_matching_order(SelectedOrder &selected_order,
                               ActiveOrder &active_order,
                               afterhours::EntityID order_id) const {
    selected_order.order_id.reset_if_matching_order(order_id);
    active_order.order_id.reset_if_matching_order(order_id);
  }

  void for_each_with(afterhours::Entity &order_entity, Order &order, OrderWorkflow &workflow,
                     float) override {
    SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
    ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

    afterhours::EntityID order_id = order_entity.id;

    if (!order_entity.hasTag(GameTag::IsInProgressOrder)) {
      reset_if_matching_order(selected_order, active_order, order_id);
      return;
    }

    int order_index = afterhours::EntityQuery()
                                  .whereID(order_entity.id)
                                  .gen_first_as<OrderSlot>().index;

    if (selected_order.order_id.is_matching_order(order_id)) {
      if (game_input::IsKeyPressed(raylib::KEY_ESCAPE)) {
        reset_if_matching_order(selected_order, active_order, order_id);
        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
        return;
      }
    }

    for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
      if (!game_input::IsKeyPressed(key)) {
        continue;
      }

      int pressed_index = key - raylib::KEY_ONE;
      if (pressed_index != order_index) {
        continue;
      }

      // Handle order opening - advance from Incoming_Arrived to Requesting_NeedsInput
      if (workflow.state == OrderState::Incoming_Arrived && kind_of(workflow.state) == OrderStateKind::Input) {
        workflow.state = OrderState::Requesting_NeedsInput;
        workflow.time_in_state = 0.0f;
        order.has_been_selected = true;
        set_selected_and_active_order(order_id);
        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
        log_info("Order {} opened and ready for input (Incoming -> Requesting)", static_cast<unsigned long long>(order_id));
        return;
      }

      // Handle order selection for already active orders
      if (kind_of(workflow.state) == OrderStateKind::Input) {
        set_selected_and_active_order(order_id);
        return;
      }

      // For already active orders, just select them
      if (kind_of(workflow.state) == OrderStateKind::Input) {
        set_selected_and_active_order(order_id);
      }
      return;
    }
  }
};
