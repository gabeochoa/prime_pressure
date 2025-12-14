#pragma once

#include "../components.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include <afterhours/ah.h>

struct UpdateOrderWorkflowSystem : afterhours::System<OrderWorkflow, Order, OrderRequiredCounts, OrderReceivedCounts> {
  void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow, Order &order,
                     OrderRequiredCounts &required_counts, OrderReceivedCounts &received_counts, float dt) override {
    // Advance processing states if complete
    OrderState new_state = advance_if_complete(workflow.state, dt, workflow.time_in_state,
                                              order, required_counts.counts, received_counts.counts);

    if (new_state == workflow.state) return;

    log_info("Order {} advanced from state {} to {}",
             static_cast<unsigned long long>(entity.id),
             static_cast<int>(workflow.state),
             static_cast<int>(new_state));
    workflow.state = new_state;
    workflow.time_in_state = 0.0f;
  }
};