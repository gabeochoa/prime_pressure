#pragma once

#include <afterhours/ah.h>

#include "../components.h"

struct ManageInProgressOrderTagSystem : afterhours::System<Order> {
    bool should_run(float) const override { return true; }
    void for_each_with(afterhours::Entity &order_entity, Order &,
                       float) override {
        bool has_valid_slot =
            afterhours::EntityQuery()
                .whereID(order_entity.id)
                .whereHasComponent<OrderSlot>()
                .whereLambda([](const afterhours::Entity &entity) {
                    const OrderSlot &slot = entity.get<OrderSlot>();
                    return slot.index >= 0;
                })
                .has_values();

        order_entity.setTag(GameTag::IsInProgressOrder, has_valid_slot);

        // Store debug information
        DebugOverlay &debug_overlay = get_singleton_as<DebugOverlay>();
        if (debug_overlay.enabled) {
            debug_overlay.debug_lines.push_back(
                "Order " +
                std::to_string(
                    static_cast<unsigned long long>(order_entity.id)) +
                " has_valid_slot=" + (has_valid_slot ? "true" : "false") +
                ", setting IsInProgressOrder tag");
        }
    }
};
