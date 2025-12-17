#pragma once

#include <afterhours/ah.h>

#include "../components.h"
#include "../input_wrapper.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include "../testing/test_input.h"

struct ProcessBoxingInputSystem : afterhours::System<Order, OrderWorkflow> {
    bool should_run(float) const override {
        return true;  // Always run since all views are visible
    }

    void for_each_with(afterhours::Entity &order_entity, Order &order,
                       OrderWorkflow &workflow, float) override {
        const ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

        if (!active_order.order_id.is_matching_order(order_entity.id)) {
            return;
        }

        // Only advance if we're in the Boxing macro state
        if (macro_state_of(workflow.state) != OrderMacroState::Boxing) {
            return;
        }

        afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::get_singleton<BoxingProgress>();
        BoxingProgress &boxing_progress =
            boxing_progress_entity.get<BoxingProgress>();

        // If we're in boxing state but boxing_progress isn't set up for this
        // order, initialize it
        if (!boxing_progress.order_id.has_value() ||
            boxing_progress.order_id.value() != order_entity.id) {
            start_boxing(boxing_progress, order_entity.id);
        }

        // Check for key presses first to avoid unnecessary work
        bool f_pressed = game_input::IsKeyPressed(raylib::KEY_F);
        bool p_pressed = game_input::IsKeyPressed(raylib::KEY_P);
        bool t_pressed = game_input::IsKeyPressed(raylib::KEY_T);
        bool s_pressed = game_input::IsKeyPressed(raylib::KEY_S);
        bool b_pressed = game_input::IsKeyPressed(raylib::KEY_B);

        if (!f_pressed && !p_pressed && !t_pressed && !s_pressed &&
            !b_pressed) {
            return;  // No boxing input this frame
        }

        // Handle boxing input based on current state

        // Handle boxing actions using the internal helper which updates
        // boxing_progress
        handle_boxing_actions(boxing_progress, f_pressed, p_pressed, t_pressed,
                              s_pressed, b_pressed);

        // Sync workflow state with boxing_progress state
        // Check if boxing_progress state has advanced and update workflow
        // accordingly
        switch (boxing_progress.state) {
            case BoxingState::FoldBox:
                if (workflow.state != OrderState::Boxing_FoldBox) {
                    workflow.state = OrderState::Boxing_FoldBox;
                    workflow.time_in_state = 0.0f;
                }
                break;
            case BoxingState::PutItems:
                if (workflow.state != OrderState::Boxing_PutItems) {
                    workflow.state = OrderState::Boxing_PutItems;
                    workflow.time_in_state = 0.0f;
                }
                break;
            case BoxingState::Fold:
                if (workflow.state != OrderState::Boxing_Fold) {
                    workflow.state = OrderState::Boxing_Fold;
                    workflow.time_in_state = 0.0f;
                }
                break;
            case BoxingState::Tape:
                if (workflow.state != OrderState::Boxing_Tape) {
                    workflow.state = OrderState::Boxing_Tape;
                    workflow.time_in_state = 0.0f;
                }
                break;
            case BoxingState::Ship:
                if (workflow.state != OrderState::Boxing_Ship) {
                    workflow.state = OrderState::Boxing_Ship;
                    workflow.time_in_state = 0.0f;
                }
                break;
            case BoxingState::None:
                // Boxing is complete, no action needed here
                break;
        }
    }

   private:
    void cleanup_boxing_items(BoxingProgress &boxing_progress) const {
        for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
            auto opt_entity = afterhours::EntityHelper::getEntityForID(item_id);
            if (opt_entity.has_value()) {
                opt_entity.value()->cleanup = true;
            }
        }
        boxing_progress.boxing_items.clear();
    }

    void finish_shipping(BoxingProgress &boxing_progress, Order &order) const {
        cleanup_boxing_items(boxing_progress);
        boxing_progress.order_id.reset();
        boxing_progress.state = BoxingState::None;
        boxing_progress.items_placed = 0;
    }

    void create_boxing_items(
        BoxingProgress &boxing_progress,
        const std::vector<ItemType> &items_to_create) const {
        for (ItemType item_type : items_to_create) {
            afterhours::Entity &boxing_item_entity =
                afterhours::EntityHelper::createEntity();
            BoxingItemStatus &boxing_item =
                boxing_item_entity.addComponent<BoxingItemStatus>();
            boxing_item.type = item_type;
            boxing_item.is_placed = false;
            boxing_progress.boxing_items.push_back(boxing_item_entity.id);
        }
    }

    void start_boxing(BoxingProgress &boxing_progress,
                      afterhours::EntityID order_id) const {
        Order *order_ptr = nullptr;
        for (afterhours::Entity &entity :
             afterhours::EntityQuery()
                 .whereID(order_id)
                 .whereHasComponent<Order>()
                 .whereLambda([](const afterhours::Entity &entity) {
                     const Order &o = entity.get<Order>();
                     return all_items_ready(o) && !o.is_shipped();
                 })
                 .gen()) {
            order_ptr = &entity.get<Order>();
            break;
        }

        if (!order_ptr) {
            return;
        }

        std::vector<ItemType> items_to_create = order_ptr->ready_items;

        cleanup_boxing_items(boxing_progress);
        create_boxing_items(boxing_progress, items_to_create);

        boxing_progress.order_id = order_id;
        boxing_progress.state = BoxingState::FoldBox;
        boxing_progress.items_placed = 0;
    }

    void handle_put_items(BoxingProgress &boxing_progress) const {
        bool item_placed = false;
        for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
            if (item_placed) {
                break;
            }
            BoxingItemStatus *boxing_item = nullptr;
            for (afterhours::Entity &entity :
                 afterhours::EntityQuery()
                     .whereID(item_id)
                     .whereHasComponent<BoxingItemStatus>()
                     .whereLambda([](const afterhours::Entity &entity) {
                         const BoxingItemStatus &bi =
                             entity.get<BoxingItemStatus>();
                         return !bi.is_placed;
                     })
                     .gen()) {
                boxing_item = &entity.get<BoxingItemStatus>();
                break;
            }

            if (boxing_item != nullptr) {
                boxing_item->is_placed = true;
                boxing_progress.items_placed++;
                item_placed = true;
                int total_items =
                    static_cast<int>(boxing_progress.boxing_items.size());
                if (boxing_progress.items_placed >= total_items) {
                    boxing_progress.state = BoxingState::Fold;
                }
            }
        }
    }

    void handle_boxing_actions(BoxingProgress &boxing_progress, bool f_pressed,
                               bool p_pressed, bool t_pressed, bool s_pressed,
                               bool b_pressed) const {
        if (!boxing_progress.order_id.has_value()) {
            return;
        }

        if (!f_pressed && !p_pressed && !t_pressed && !s_pressed &&
            !b_pressed) {
            return;  // No boxing input this frame
        }

        for (Order &order : afterhours::EntityQuery()
                                .whereID(boxing_progress.order_id.value())
                                .whereHasComponent<Order>()
                                .gen_as<Order>()) {
            switch (boxing_progress.state) {
                case BoxingState::None:
                    // No boxing in progress, nothing to do
                    break;
                case BoxingState::FoldBox:
                    if (b_pressed) {
                        boxing_progress.state = BoxingState::PutItems;
                    }
                    break;

                case BoxingState::PutItems:
                    if (p_pressed) {
                        handle_put_items(boxing_progress);
                    }
                    break;

                case BoxingState::Fold:
                    if (f_pressed) {
                        boxing_progress.state = BoxingState::Tape;
                    }
                    break;

                case BoxingState::Tape:
                    if (t_pressed) {
                        boxing_progress.state = BoxingState::Ship;
                    }
                    break;

                case BoxingState::Ship:
                    if (s_pressed) {
                        finish_shipping(boxing_progress, order);
                        // Immediately advance workflow state to stamping phase
                        workflow.state = OrderState::Shipped_Stamp0;
                        workflow.time_in_state = 0.0f;
                    }
                    break;

                default:
                    // No action for None or invalid states
                    log_warn("Unhandled boxing progress state: {}",
                             static_cast<int>(boxing_progress.state));
                    break;
            }
            break;  // Exit after processing one order
        }
    }
};
