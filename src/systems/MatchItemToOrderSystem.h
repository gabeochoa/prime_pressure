#pragma once

#include <afterhours/ah.h>

#include "../components.h"
#include "../eq.h"
#include "../order_components.h"
#include "../ui_constants.h"

static std::string to_lower(const std::string &str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        result +=
            static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

inline bool mark_conveyor_item_as_moving(afterhours::EntityID order_id,
                                         ItemType item_type) {
    auto conveyor_opt = EQ().whereHasComponent<ConveyorItem>()
                            .whereHasTag(GameTag::IsOnConveyor)
                            .whereHasOrderID(order_id)
                            .whereLambda([&](const afterhours::Entity &entity) {
                                const ConveyorItem &conveyor_item =
                                    entity.get<ConveyorItem>();
                                return conveyor_item.type == item_type &&
                                       !conveyor_item.is_moving;
                            })
                            .gen_first();
    if (conveyor_opt.has_value()) {
        ConveyorItem &conveyor_item = conveyor_opt->get<ConveyorItem>();
        conveyor_item.is_moving = true;
        return true;
    }
    return false;
}

struct MatchItemToOrderSystem
    : afterhours::System<Order, OrderWorkflow, OrderRequiredCounts,
                         OrderRequestedCounts,
                         afterhours::tags::All<GameTag::IsSelectedOrder>> {
    bool should_run(float) const override { return true; }

    void for_each_with(afterhours::Entity &order_entity, Order &order,
                       OrderWorkflow &workflow,
                       OrderRequiredCounts &required_counts,
                       OrderRequestedCounts &requested_counts, float) override {
        afterhours::Entity &buffer_entity =
            afterhours::EntityHelper::get_singleton<TypingBuffer>();
        TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();

        if (buffer.buffer.empty()) {
            buffer.status = TypingStatus::Idle;
            return;
        }

        if (buffer.buffer.length() != 1) {
            buffer.status = TypingStatus::Typing;
            return;
        }

        char typed_key = buffer.buffer[0];

        // Check if we're in the correct requesting state
        if (workflow.state != OrderState::Requesting_NeedsInput) {
            return;
        }

        // Check if the typed key matches an available item
        for (const auto &[item_type, required_count] : required_counts.counts) {
            auto requested_it = requested_counts.counts.find(item_type);
            int requested_count =
                (requested_it != requested_counts.counts.end())
                    ? requested_it->second
                    : 0;

            if (requested_count >= required_count) {
                continue;
            }

            auto key_it = ITEM_KEY_MAP.find(item_type);
            if (key_it == ITEM_KEY_MAP.end()) {
                continue;
            }

            char item_key = key_it->second;
            if (typed_key != item_key) {
                continue;
            }

            // Request the item
            requested_counts.counts[item_type]++;
            order.selected_items.push_back(
                item_type);  // Keep for compatibility
            mark_conveyor_item_as_moving(order_entity.id, item_type);

            // Check if all items are now requested
            bool all_requested = true;
            for (const auto &[check_type, check_required] :
                 required_counts.counts) {
                auto check_requested_it =
                    requested_counts.counts.find(check_type);
                int check_requested =
                    (check_requested_it != requested_counts.counts.end())
                        ? check_requested_it->second
                        : 0;
                if (check_requested < check_required) {
                    all_requested = false;
                    break;
                }
            }

            if (all_requested) {
                workflow.state = OrderState::Requesting_AllRequested;
                workflow.time_in_state = 0.0f;
                log_info(
                    "Order {} all items requested, advancing to AllRequested",
                    static_cast<unsigned long long>(order_entity.id));
            }

            buffer.buffer.clear();
            buffer.status = TypingStatus::Match;
            buffer.status_time = 0.0f;
            buffer.last_input_time = 0.0f;
            return;
        }

        // Invalid key - transition to error state
        workflow.state = OrderState::Requesting_InputError;
        workflow.time_in_state = 0.0f;
        log_info("Order {} invalid input, advancing to InputError",
                 static_cast<unsigned long long>(order_entity.id));

        buffer.status = TypingStatus::Error;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
    }
};
