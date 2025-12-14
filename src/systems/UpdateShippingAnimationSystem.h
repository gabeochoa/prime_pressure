#pragma once

#include <afterhours/ah.h>

#include "../components.h"
#include "../order_components.h"

struct UpdateShippingAnimationSystem : afterhours::System<> {
    bool should_run(float) const override { return true; }

    void once(float dt) override {
        afterhours::Entity &animation_entity =
            afterhours::EntityHelper::get_singleton<ShippingAnimation>();
        ShippingAnimation &animation =
            animation_entity.get<ShippingAnimation>();

        if (!animation.is_active) {
            return;
        }

        // Update animation time
        animation.animation_time += dt;

        // Check if animation is complete
        if (animation.animation_time < animation.animation_duration) {
            return;
        }

        log_info("Shipping animation completed for order {}",
                 animation.order_id.value_or(0));
        // Advance order to completed state
        if (animation.order_id.has_value()) {
            int anim_order_id = animation.order_id.value();

            for (OrderWorkflow &workflow :
                 afterhours::EntityQuery()
                     .whereID(anim_order_id)
                     .whereHasComponent<OrderWorkflow>()
                     .whereLambda([](const afterhours::Entity &e) {
                         return e.get<OrderWorkflow>().state ==
                                OrderState::Complete_CloseoutDelay;
                     })
                     .gen_as<OrderWorkflow>()) {
                workflow.state = OrderState::Complete_ClosedOut;
                workflow.time_in_state = 0.0f;
            }

            // Clear boxing progress for this order
            afterhours::Entity &boxing_progress_entity =
                afterhours::EntityHelper::get_singleton<BoxingProgress>();
            BoxingProgress &boxing_progress =
                boxing_progress_entity.get<BoxingProgress>();
            if (boxing_progress.order_id.has_value() &&
                boxing_progress.order_id.value() == anim_order_id) {
                boxing_progress.order_id.reset();
                boxing_progress.state = BoxingState::None;
                boxing_progress.items_placed = 0;
            }
        }

        // Reset animation
        animation.is_active = false;
        animation.animation_time = 0.0f;
        animation.order_id.reset();
    }
};
