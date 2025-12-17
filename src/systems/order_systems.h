#pragma once

#include <afterhours/ah.h>

#include <magic_enum/magic_enum.hpp>
#include <random>

#include "../components.h"
#include "../eq.h"
#include "../input_wrapper.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include "../string_utils.h"
#include "../testing/test_input.h"
#include "../ui_constants.h"

// Utility functions for order state management

// Maps OrderState enum values to OrderMicroTag values for microstates
inline OrderMicroTag microstate_tag_of(OrderState state) {
    switch (state) {
        case OrderState::Incoming_Arrived:
            return OrderMicroTag::Incoming_Arrived;
        case OrderState::Incoming_Backlogged:
            return OrderMicroTag::Incoming_Backlogged;
        case OrderState::Opened_Active:
            return OrderMicroTag::Opened_Active;
        case OrderState::Opened_Inactive:
            return OrderMicroTag::Opened_Inactive;
        case OrderState::Requesting_NeedsInput:
            return OrderMicroTag::Requesting_NeedsInput;
        case OrderState::Requesting_InputError:
            return OrderMicroTag::Requesting_InputError;
        case OrderState::Requesting_AllRequested:
            return OrderMicroTag::Requesting_AllRequested;
        case OrderState::Receiving_OnConveyorWaiting:
            return OrderMicroTag::Receiving_OnConveyorWaiting;
        case OrderState::Receiving_OnConveyorMoving:
            return OrderMicroTag::Receiving_OnConveyorMoving;
        case OrderState::Receiving_ReceivedToReady:
            return OrderMicroTag::Receiving_ReceivedToReady;
        case OrderState::ReadyToBox_Staged:
            return OrderMicroTag::ReadyToBox_Staged;
        case OrderState::ReadyToBox_Queued:
            return OrderMicroTag::ReadyToBox_Queued;
        case OrderState::Boxing_FoldBox:
            return OrderMicroTag::Boxing_FoldBox;
        case OrderState::Boxing_PutItems:
            return OrderMicroTag::Boxing_PutItems;
        case OrderState::Boxing_Fold:
            return OrderMicroTag::Boxing_Fold;
        case OrderState::Boxing_Tape:
            return OrderMicroTag::Boxing_Tape;
        case OrderState::Boxing_Ship:
            return OrderMicroTag::Boxing_Ship;
        case OrderState::Shipped_Stamp0:
            return OrderMicroTag::Shipped_Stamp0;
        case OrderState::Shipped_Stamp1:
            return OrderMicroTag::Shipped_Stamp1;
        case OrderState::Shipped_Stamp2:
            return OrderMicroTag::Shipped_Stamp2;
        case OrderState::Complete_CloseoutDelay:
            return OrderMicroTag::Complete_CloseoutDelay;
        case OrderState::Complete_ClosedOut:
            return OrderMicroTag::Complete_ClosedOut;
        default:
            log_error("Unknown OrderState in microstate_tag_of: {}",
                      static_cast<int>(state));
            return OrderMicroTag::Incoming_Arrived;
    }
}

// Maps OrderMacroState enum values to OrderMacroTag values for macrostates
inline OrderMacroTag macrostate_tag_of(OrderMacroState state) {
    switch (state) {
        case OrderMacroState::Incoming:
            return OrderMacroTag::Incoming;
        case OrderMacroState::Opened:
            return OrderMacroTag::Opened;
        case OrderMacroState::RequestingItems:
            return OrderMacroTag::RequestingItems;
        case OrderMacroState::ReceivingItems:
            return OrderMacroTag::ReceivingItems;
        case OrderMacroState::ReadyToBox:
            return OrderMacroTag::ReadyToBox;
        case OrderMacroState::Boxing:
            return OrderMacroTag::Boxing;
        case OrderMacroState::Shipped:
            return OrderMacroTag::Shipped;
        case OrderMacroState::Complete:
            return OrderMacroTag::Complete;
        default:
            log_error("Unknown OrderMacroState in macrostate_tag_of: {}",
                      static_cast<int>(state));
            return OrderMacroTag::Incoming;
    }
}

// Utility function to mark conveyor items as moving
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
    if (!conveyor_opt.has_value()) {
        return false;
    }
    ConveyorItem &conveyor_item = conveyor_opt->get<ConveyorItem>();
    conveyor_item.is_moving = true;
    return true;
}

// Utility function to disable all microstate tags
inline void disable_all_microstate_tags(afterhours::Entity &entity) {
    entity.disableTag(OrderMicroTag::Incoming_Arrived);
    entity.disableTag(OrderMicroTag::Incoming_Backlogged);
    entity.disableTag(OrderMicroTag::Opened_Active);
    entity.disableTag(OrderMicroTag::Opened_Inactive);
    entity.disableTag(OrderMicroTag::Requesting_NeedsInput);
    entity.disableTag(OrderMicroTag::Requesting_InputError);
    entity.disableTag(OrderMicroTag::Requesting_AllRequested);
    entity.disableTag(OrderMicroTag::Receiving_OnConveyorWaiting);
    entity.disableTag(OrderMicroTag::Receiving_OnConveyorMoving);
    entity.disableTag(OrderMicroTag::Receiving_ReceivedToReady);
    entity.disableTag(OrderMicroTag::ReadyToBox_Staged);
    entity.disableTag(OrderMicroTag::ReadyToBox_Queued);
    entity.disableTag(OrderMicroTag::Boxing_FoldBox);
    entity.disableTag(OrderMicroTag::Boxing_PutItems);
    entity.disableTag(OrderMicroTag::Boxing_Fold);
    entity.disableTag(OrderMicroTag::Boxing_Tape);
    entity.disableTag(OrderMicroTag::Boxing_Ship);
    entity.disableTag(OrderMicroTag::Shipped_Stamp0);
    entity.disableTag(OrderMicroTag::Shipped_Stamp1);
    entity.disableTag(OrderMicroTag::Shipped_Stamp2);
    entity.disableTag(OrderMicroTag::Complete_CloseoutDelay);
    entity.disableTag(OrderMicroTag::Complete_ClosedOut);
}

// Utility function to disable all macrostate tags
inline void disable_all_macrostate_tags(afterhours::Entity &entity) {
    entity.disableTag(OrderMacroTag::Incoming);
    entity.disableTag(OrderMacroTag::Opened);
    entity.disableTag(OrderMacroTag::RequestingItems);
    entity.disableTag(OrderMacroTag::ReceivingItems);
    entity.disableTag(OrderMacroTag::ReadyToBox);
    entity.disableTag(OrderMacroTag::Boxing);
    entity.disableTag(OrderMacroTag::Shipped);
    entity.disableTag(OrderMacroTag::Complete);
}

// Utility function to count enabled microstate tags
inline int count_enabled_microstate_tags(const afterhours::Entity &entity) {
    int count = 0;
    if (entity.hasTag(OrderMicroTag::Incoming_Arrived)) count++;
    if (entity.hasTag(OrderMicroTag::Incoming_Backlogged)) count++;
    if (entity.hasTag(OrderMicroTag::Opened_Active)) count++;
    if (entity.hasTag(OrderMicroTag::Opened_Inactive)) count++;
    if (entity.hasTag(OrderMicroTag::Requesting_NeedsInput)) count++;
    if (entity.hasTag(OrderMicroTag::Requesting_InputError)) count++;
    if (entity.hasTag(OrderMicroTag::Requesting_AllRequested)) count++;
    if (entity.hasTag(OrderMicroTag::Receiving_OnConveyorWaiting)) count++;
    if (entity.hasTag(OrderMicroTag::Receiving_OnConveyorMoving)) count++;
    if (entity.hasTag(OrderMicroTag::Receiving_ReceivedToReady)) count++;
    if (entity.hasTag(OrderMicroTag::ReadyToBox_Staged)) count++;
    if (entity.hasTag(OrderMicroTag::ReadyToBox_Queued)) count++;
    if (entity.hasTag(OrderMicroTag::Boxing_FoldBox)) count++;
    if (entity.hasTag(OrderMicroTag::Boxing_PutItems)) count++;
    if (entity.hasTag(OrderMicroTag::Boxing_Fold)) count++;
    if (entity.hasTag(OrderMicroTag::Boxing_Tape)) count++;
    if (entity.hasTag(OrderMicroTag::Boxing_Ship)) count++;
    if (entity.hasTag(OrderMicroTag::Shipped_Stamp0)) count++;
    if (entity.hasTag(OrderMicroTag::Shipped_Stamp1)) count++;
    if (entity.hasTag(OrderMicroTag::Shipped_Stamp2)) count++;
    if (entity.hasTag(OrderMicroTag::Complete_CloseoutDelay)) count++;
    if (entity.hasTag(OrderMicroTag::Complete_ClosedOut)) count++;
    return count;
}

// Utility function to count enabled macrostate tags
inline int count_enabled_macrostate_tags(const afterhours::Entity &entity) {
    int count = 0;
    if (entity.hasTag(OrderMacroTag::Incoming)) count++;
    if (entity.hasTag(OrderMacroTag::Opened)) count++;
    if (entity.hasTag(OrderMacroTag::RequestingItems)) count++;
    if (entity.hasTag(OrderMacroTag::ReceivingItems)) count++;
    if (entity.hasTag(OrderMacroTag::ReadyToBox)) count++;
    if (entity.hasTag(OrderMacroTag::Boxing)) count++;
    if (entity.hasTag(OrderMacroTag::Shipped)) count++;
    if (entity.hasTag(OrderMacroTag::Complete)) count++;
    return count;
}

// Order Systems

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

struct UpdateOrderWorkflowSystem
    : afterhours::System<OrderWorkflow, Order, OrderRequiredCounts,
                         OrderReceivedCounts> {
    void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow,
                       Order &order, OrderRequiredCounts &required_counts,
                       OrderReceivedCounts &received_counts,
                       float dt) override {
        // Advance processing states if complete
        OrderState new_state = advance_if_complete(
            workflow.state, dt, workflow.time_in_state, order,
            required_counts.counts, received_counts.counts);

        if (new_state == workflow.state) return;

        log_info("Order {} advanced from state {} to {}",
                 static_cast<unsigned long long>(entity.id),
                 static_cast<int>(workflow.state), static_cast<int>(new_state));
        workflow.state = new_state;
        workflow.time_in_state = 0.0f;
    }
};

struct ManageOrderMicrostateTagsSystem : afterhours::System<OrderWorkflow> {
    void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow,
                       float) override {
        disable_all_microstate_tags(entity);
        entity.enableTag(microstate_tag_of(workflow.state));
    }
};

struct ManageOrderMacrostateTagsSystem : afterhours::System<OrderWorkflow> {
    void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow,
                       float) override {
        disable_all_macrostate_tags(entity);
        // Enable the correct macrostate tag
        entity.enableTag(macrostate_tag_of(macro_state_of(workflow.state)));
    }
};

struct ValidateOrderStateTagsSystem : afterhours::System<OrderWorkflow> {
    void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow,
                       float) override {
        if (STATE_TABLE.find(workflow.state) == STATE_TABLE.end()) {
            log_error("Invalid order state {} for entity {}",
                      static_cast<int>(workflow.state),
                      static_cast<unsigned long long>(entity.id));
            return;
        }

        // Simple validation - ensure exactly one microstate and one macrostate
        // tag are enabled
        int micro_count = count_enabled_microstate_tags(entity);
        int macro_count = count_enabled_macrostate_tags(entity);

        if (micro_count != 1 || macro_count != 1) {
            log_error(
                "Order {} has invalid tag counts: micro={}, macro={} (should "
                "be 1 each)",
                static_cast<unsigned long long>(entity.id), micro_count,
                macro_count);
        }
    }
};

struct ManageSelectedOrderTagSystem : afterhours::System<Order> {
   private:
    const SelectedOrder *selected_order_ptr = nullptr;

   public:
    void once(float) override {
        selected_order_ptr = &get_singleton_as<SelectedOrder>();
    }

    void for_each_with(afterhours::Entity &order_entity, Order &,
                       float) override {
        if (!selected_order_ptr) return;

        order_entity.setTag(GameTag::IsSelectedOrder,
                            selected_order_ptr->order_id.order_id.has_value() &&
                                selected_order_ptr->order_id.order_id.value() ==
                                    order_entity.id);
    }
};

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
    }
};

struct ProcessOrderSelectionSystem : afterhours::System<Order, OrderWorkflow> {
    bool should_run(float) const override { return true; }

    void for_each_with(afterhours::Entity &order_entity, Order &order,
                       OrderWorkflow &workflow, float) override {
        afterhours::EntityID order_id = order_entity.id;

        if (!order_entity.hasTag(GameTag::IsInProgressOrder)) {
            handle_non_progress_order(order_id);
            return;
        }

        int order_index = get_order_index(order_entity);
        if (handle_escape_key_if_selected(order_id)) {
            return;
        }

        handle_number_key_selection(order_entity, order, workflow, order_id,
                                    order_index);
    }

   private:
    void handle_non_progress_order(afterhours::EntityID order_id) {
        SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
        ActiveOrder &active_order = get_singleton_as<ActiveOrder>();
        selected_order.order_id.reset_if_matching_order(order_id);
        active_order.order_id.reset_if_matching_order(order_id);
    }

    int get_order_index(const afterhours::Entity &order_entity) {
        return afterhours::EntityQuery()
            .whereID(order_entity.id)
            .gen_first_as<OrderSlot>()
            .index;
    }

    void log_debug_info(afterhours::EntityID order_id, int order_index,
                        OrderState state) {
        DebugOverlay &debug_overlay = get_singleton_as<DebugOverlay>();
        if (debug_overlay.enabled) {
            debug_overlay.debug_lines.push_back(
                "Processing order " +
                std::to_string(static_cast<unsigned long long>(order_id)) +
                ", index " + std::to_string(order_index) + ", state " +
                std::to_string(static_cast<int>(state)));
        }
    }

    bool handle_escape_key_if_selected(afterhours::EntityID order_id) {
        SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
        if (!selected_order.order_id.is_matching_order(order_id)) {
            return false;
        }

        if (!game_input::IsKeyPressed(raylib::KEY_ESCAPE)) {
            return false;
        }

        selected_order.order_id.order_id.reset();
        ActiveOrder &active_order = get_singleton_as<ActiveOrder>();
        active_order.order_id.order_id.reset();

        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;

        return true;
    }

    void handle_number_key_selection(afterhours::Entity &order_entity,
                                     Order &order, OrderWorkflow &workflow,
                                     afterhours::EntityID order_id,
                                     int order_index) {
        for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
            if (!game_input::IsKeyPressed(key)) {
                continue;
            }

            int pressed_index = key - raylib::KEY_ONE;
            if (pressed_index != order_index) {
                continue;
            }

            handle_order_activation(order_entity, order, workflow, order_id,
                                    key);
            break;  // Only handle one key press per frame
        }
    }

    void handle_order_activation(afterhours::Entity &order_entity, Order &order,
                                 OrderWorkflow &workflow,
                                 afterhours::EntityID order_id, int key) {
        SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
        ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

        // Handle order opening - advance from Incoming_Arrived to
        // Requesting_NeedsInput
        if (workflow.state == OrderState::Incoming_Arrived &&
            kind_of(workflow.state) == OrderStateKind::Input) {
            workflow.state = OrderState::Requesting_NeedsInput;
            workflow.time_in_state = 0.0f;
            order.has_been_selected = true;
            selected_order.order_id.set_order_id(order_id);
            active_order.order_id.set_order_id(order_id);

            clear_typing_buffer();
            log_order_opened(order_id);
            return;
        }

        // Handle order selection for already active orders
        if (kind_of(workflow.state) == OrderStateKind::Input) {
            selected_order.order_id.set_order_id(order_id);
            active_order.order_id.set_order_id(order_id);
        }
    }

    void clear_typing_buffer() {
        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
    }

    void log_order_opened(afterhours::EntityID order_id) {
        DebugOverlay &debug_overlay = get_singleton_as<DebugOverlay>();
        if (debug_overlay.enabled) {
            debug_overlay.debug_lines.push_back(
                "Order " +
                std::to_string(static_cast<unsigned long long>(order_id)) +
                " opened and ready for input");
        }
    }
};

struct GenerateOrdersSystem : afterhours::System<> {
    static constexpr auto simple_items = magic_enum::enum_values<ItemType>();
    static constexpr int MAX_IN_PROGRESS_ORDERS = 3;

    float time_since_last_order = 0.0f;
    float order_interval = ui_constants::ORDER_GENERATION_INTERVAL;
    bool initial_order_generated = false;

    void once(float dt) override {
        if (dt <= 0.0f) {
            return;
        }

        if (test_input::test_mode && !initial_order_generated &&
            count_in_progress_orders() == 0) {
            int free_slot = find_free_slot();
            if (free_slot < 0) {
                return;
            }
            generate_order_to_slot(free_slot);
            initial_order_generated = true;
            return;
        }

        time_since_last_order += dt;

        if (count_in_progress_orders() >= MAX_IN_PROGRESS_ORDERS) {
            return;
        }

        if (time_since_last_order < order_interval) {
            return;
        }

        int free_slot = find_free_slot();
        if (free_slot >= 0) {
            generate_order_to_slot(free_slot);
        }
        time_since_last_order = 0.0f;
    }

   private:
    size_t count_in_progress_orders() const {
        return afterhours::EntityQuery()
            .whereHasComponent<OrderSlot>()
            .whereLambda([](const afterhours::Entity &entity) {
                const OrderSlot &slot = entity.get<OrderSlot>();
                return slot.index >= 0;
            })
            .gen_count();
    }

    int find_free_slot() const {
        for (int i = 0; i < MAX_IN_PROGRESS_ORDERS; ++i) {
            bool slot_used =
                afterhours::EntityQuery()
                    .whereHasComponent<OrderSlot>()
                    .whereLambda([i](const afterhours::Entity &entity) {
                        const OrderSlot &slot = entity.get<OrderSlot>();
                        return slot.index == i;
                    })
                    .has_values();

            if (!slot_used) {
                return i;
            }
        }
        return -1;
    }

    void generate_order_to_slot(int slot_index) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> item_count_dist(1, 3);
        std::uniform_int_distribution<> item_index_dist(
            0, static_cast<int>(simple_items.size() - 1));

        int item_count = item_count_dist(gen);
        std::vector<ItemType> order_items;

        for (int i = 0; i < item_count; ++i) {
            ItemType selected_item = simple_items[item_index_dist(gen)];
            order_items.push_back(selected_item);
        }

        afterhours::Entity &order_entity =
            afterhours::EntityHelper::createEntity();
        Order &order = order_entity.addComponent<Order>();
        order.items = order_items;

        OrderSlot &slot = order_entity.addComponent<OrderSlot>();
        slot.index = slot_index;

        OrderWorkflow &workflow = order_entity.addComponent<OrderWorkflow>();
        workflow.state = OrderState::Incoming_Arrived;
        workflow.time_in_state = 0.0f;

        OrderRequiredCounts &required_counts =
            order_entity.addComponent<OrderRequiredCounts>();
        required_counts.counts = count_items(order_items);

        order_entity.addComponent<OrderRequestedCounts>();
        order_entity.addComponent<OrderReceivedCounts>();
        order_entity.addComponent<OrderBoxedCounts>();
    }
};

struct DebugOrderWorkflowSystem : afterhours::System<OrderWorkflow> {
    void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow,
                       float) override {
        static float last_log_time = 0.0f;
        float current_time = static_cast<float>(raylib::GetTime());

        // Log every 2 seconds
        if (current_time - last_log_time > 2.0f) {
            log_info(
                "Order {}: macro={}, micro={}, kind={}, time_in_state={:.2f}",
                static_cast<unsigned long long>(entity.id),
                static_cast<int>(macro_state_of(workflow.state)),
                static_cast<int>(workflow.state),
                static_cast<int>(kind_of(workflow.state)),
                workflow.time_in_state);
            last_log_time = current_time;
        }
    }
};

struct ProcessOrderTabbingSystem : afterhours::System<> {
    bool should_run(float) const override {
        const ActiveView &active_view = get_singleton_as<ActiveView>();
        return active_view.current_view != ViewState::Cutscene;
    }

    void once(float) override {
        if (!game_input::IsKeyPressed(raylib::KEY_TAB)) {
            return;
        }

        ActiveOrder &active_order = get_singleton_as<ActiveOrder>();
        SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();

        // Collect in-progress, unshipped orders for cycling
        std::vector<afterhours::EntityID> selectable_orders;
        for (const afterhours::Entity &entity :
             afterhours::EntityQuery()
                 .whereHasComponent<OrderSlot>()
                 .whereHasComponent<Order>()
                 .whereLambda([](const afterhours::Entity &e) {
                     const OrderSlot &slot = e.get<OrderSlot>();
                     return slot.index >= 0 && !e.get<Order>().is_shipped();
                 })
                 .gen()) {
            selectable_orders.push_back(entity.id);
        }

        if (selectable_orders.empty()) {
            active_order.order_id.order_id.reset();
            selected_order.order_id.order_id.reset();
            return;
        }

        // Find current order index
        size_t current_index = 0;
        if (active_order.order_id.order_id.has_value()) {
            for (size_t i = 0; i < selectable_orders.size(); ++i) {
                if (selectable_orders[i] ==
                    active_order.order_id.order_id.value()) {
                    current_index = i;
                    break;
                }
            }
        }

        // Calculate next index (shift+tab goes backward)
        size_t next_index = current_index;
        if (game_input::IsKeyDown(raylib::KEY_LEFT_SHIFT) ||
            game_input::IsKeyDown(raylib::KEY_RIGHT_SHIFT)) {
            next_index = (current_index + selectable_orders.size() - 1) %
                         selectable_orders.size();
        } else {
            next_index = (current_index + 1) % selectable_orders.size();
        }

        afterhours::EntityID chosen_id = selectable_orders[next_index];

        // Update selection
        active_order.order_id.set_order_id(chosen_id);
        selected_order.order_id.set_order_id(chosen_id);

        // Mark order as selected
        for (Order &order : afterhours::EntityQuery()
                                .whereID(chosen_id)
                                .whereHasComponent<Order>()
                                .gen_as<Order>()) {
            order.has_been_selected = true;
            break;
        }

        // Clear typing buffer
        TypingBuffer &buffer = get_singleton_as<TypingBuffer>();
        buffer.buffer.clear();
        buffer.status = TypingStatus::Idle;
        buffer.status_time = 0.0f;
        buffer.last_input_time = 0.0f;
    }
};

struct MatchItemToOrderSystem
    : afterhours::System<Order, OrderWorkflow, OrderRequiredCounts,
                         OrderRequestedCounts,
                         afterhours::tags::All<GameTag::IsSelectedOrder>> {
    bool should_run(float) const override {
        const TypingBuffer &buf = get_singleton_as<TypingBuffer>();
        return !buf.buffer.empty() && buf.buffer.length() == 1;
    }

    void once(float) override {
        buffer = &get_singleton_as<TypingBuffer>();

        // Set status based on buffer state (this runs regardless of should_run
        // result)
        if (buffer->buffer.empty()) {
            buffer->status = TypingStatus::Idle;
        } else if (buffer->buffer.length() != 1) {
            buffer->status = TypingStatus::Typing;
        }
    }

    void for_each_with(afterhours::Entity &order_entity, Order &order,
                       OrderWorkflow &workflow,
                       OrderRequiredCounts &required_counts,
                       OrderRequestedCounts &requested_counts, float) override {
        if (buffer->buffer.empty()) {
            return;
        }
        char typed_key = buffer->buffer[0];

        // Only process input in the correct state
        if (workflow.state != OrderState::Requesting_NeedsInput) {
            return;
        }

        // Find matching item for the typed key
        for (const auto &[item_type, required_count] : required_counts.counts) {
            int requested_count = requested_counts.counts.count(item_type)
                                      ? requested_counts.counts.at(item_type)
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
            order.selected_items.push_back(item_type);
            mark_conveyor_item_as_moving(order_entity.id, item_type);

            // Check if all items are now requested
            bool all_requested = true;
            for (const auto &[check_type, check_required] :
                 required_counts.counts) {
                int check_requested =
                    requested_counts.counts.count(check_type)
                        ? requested_counts.counts.at(check_type)
                        : 0;
                if (check_requested < check_required) {
                    all_requested = false;
                    break;
                }
            }

            if (all_requested) {
                workflow.state = OrderState::Requesting_AllRequested;
                workflow.time_in_state = 0.0f;
                log_info("Order {} all items requested",
                         static_cast<unsigned long long>(order_entity.id));
            }

            buffer->buffer.clear();
            set_buffer_status(TypingStatus::Match);
            return;
        }

        // Invalid key - transition to error state
        workflow.state = OrderState::Requesting_InputError;
        workflow.time_in_state = 0.0f;
        log_info("Order {} invalid input",
                 static_cast<unsigned long long>(order_entity.id));

        set_buffer_status(TypingStatus::Error);
    }

   private:
    void set_buffer_status(TypingStatus status) {
        buffer->status = status;
        buffer->status_time = 0.0f;
        buffer->last_input_time = 0.0f;
    }

    TypingBuffer *buffer = nullptr;
};