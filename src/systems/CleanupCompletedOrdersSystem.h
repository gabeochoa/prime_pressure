#pragma once

#include <afterhours/ah.h>

#include "../components.h"
#include "../order_components.h"
#include "../order_state_machine.h"

struct CleanupCompletedOrdersSystem
    : afterhours::System<OrderWorkflow, OrderSlot> {
    void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow,
                       OrderSlot &slot, float dt) override {
        // Only clean up orders that are completed and have been in
        // Complete_ClosedOut state for at least 1 second
        if (workflow.state != OrderState::Complete_ClosedOut ||
            workflow.time_in_state < 1.0f) {
            return;
        }
        entity.cleanup = true;  // Mark entity for removal
    }
};