#pragma once

#include <afterhours/ah.h>

#include "../components.h"

struct ManageInProgressOrderTagSystem : afterhours::System<Order> {
    bool should_run(float) const override {
        log_info("ManageInProgressOrderTagSystem: should_run called");
        return true;
    }
    void for_each_with(afterhours::Entity &order_entity, Order &,
                       float) override {
        log_info("ManageInProgressOrderTagSystem: Processing order entity {}",
                 static_cast<unsigned long long>(order_entity.id));
        bool has_valid_slot =
            afterhours::EntityQuery()
                .whereID(order_entity.id)
                .whereHasComponent<OrderSlot>()
                .whereLambda([](const afterhours::Entity &entity) {
                    const OrderSlot &slot = entity.get<OrderSlot>();
                    return slot.index >= 0;
                })
                .has_values();

        log_info(
            "ManageInProgressOrderTagSystem: Order {} has_valid_slot={}, "
            "setting IsInProgressOrder tag",
            static_cast<unsigned long long>(order_entity.id), has_valid_slot);
        order_entity.setTag(GameTag::IsInProgressOrder, has_valid_slot);
    }
};
