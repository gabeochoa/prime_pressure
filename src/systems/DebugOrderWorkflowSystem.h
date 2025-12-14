#pragma once

#include "../components.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include <afterhours/ah.h>

// TODO: Debug system could be enhanced for better development experience
// Consider: Add debug overlay showing state names instead of enum values
// Consider: Add hotkeys to force state transitions for testing
// Consider: Add state transition history/tracing
// Consider: Add performance metrics (time spent in each state)
// Consider: Add conditional logging based on order ID or state type

struct DebugOrderWorkflowSystem : afterhours::System<OrderWorkflow> {
  void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow, float) override {
    static float last_log_time = 0.0f;
    float current_time = static_cast<float>(raylib::GetTime());

    // Log every 2 seconds
    if (current_time - last_log_time > 2.0f) {
      log_info("Order {}: macro={}, micro={}, kind={}, time_in_state={:.2f}",
               static_cast<unsigned long long>(entity.id),
               static_cast<int>(macro_state_of(workflow.state)),
               static_cast<int>(workflow.state),
               static_cast<int>(kind_of(workflow.state)),
               workflow.time_in_state);
      last_log_time = current_time;
    }
  }
};
