#pragma once

#include <afterhours/ah.h>

#include <coroutine>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "../components.h"
#include "../order_state_machine.h"
#include "../rl.h"
#include "test_input.h"

// Helper function to get key for item type (used by tests)
static char item_key_for(ItemType type) {
    switch (type) {
        case ItemType::Book:
            return 'b';
        case ItemType::Pen:
            return 'p';
        case ItemType::Mug:
            return 'm';
        case ItemType::Cup:
            return 'c';
        case ItemType::Bag:
            return 'g';
        case ItemType::Box:
            return 'x';
        case ItemType::Toy:
            return 't';
        case ItemType::Hat:
            return 'h';
        case ItemType::Key:
            return 'k';
        case ItemType::Map:
            return 'a';
    }
    return 'z';  // fallback
}

namespace test_app {
extern int frame_counter;
}

struct TestApp {
    struct promise_type {
        std::string error_message;
        bool done = false;
        int return_code = -1;  // -1 means not returned yet

        TestApp get_return_object() {
            return TestApp{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {
            try {
                std::rethrow_exception(std::current_exception());
            } catch (const std::exception &e) {
                error_message = e.what();
            }
            done = true;
        }
        void return_value(int value) {
            return_code = value;
            done = true;
        }
    };

    std::coroutine_handle<promise_type> handle;

    TestApp(std::coroutine_handle<promise_type> h) : handle(h) {
        log_info("TestApp: Created test coroutine");
    }

    TestApp(const TestApp &) = delete;
    TestApp &operator=(const TestApp &) = delete;

    TestApp(TestApp &&other) noexcept : handle(other.handle) {
        other.handle = {};
    }

    TestApp &operator=(TestApp &&other) noexcept {
        if (this != &other) {
            if (handle) {
                handle.destroy();
            }
            handle = other.handle;
            other.handle = {};
        }
        return *this;
    }

    ~TestApp() {
        if (handle) {
            handle.destroy();
        }
    }

    bool is_done() const {
        if (!handle) return true;
        return handle.promise().done;
    }

    std::string get_error() const {
        if (!handle) return "";
        return handle.promise().error_message;
    }

    bool is_successfully_completed() const {
        if (!handle) return true;
        const auto &promise = handle.promise();
        return promise.done && promise.return_code == 0 &&
               promise.error_message.empty();
    }

    int get_return_value() const {
        if (!handle) return 0;
        return handle.promise().return_code;
    }

    void resume() {
        if (handle && !handle.done()) {
            handle.resume();
        }
    }

    static void simulate_key(int key) { test_input::push_key(key); }

    static void simulate_char(char c) { test_input::push_char(c); }

    static void simulate_typing(const std::string &text) {
        for (char c : text) {
            test_input::push_char(c);
        }
    }

    struct WaitFrames {
        int target_frame;
        int start_frame;
        int slow_multiplier;

        WaitFrames(int frames)
            : target_frame(test_app::frame_counter + frames),
              start_frame(test_app::frame_counter),
              slow_multiplier(test_input::slow_test_mode ? 100 : 1) {
            if (test_input::slow_test_mode) {
                target_frame =
                    test_app::frame_counter + (frames * slow_multiplier);
            }
        }

        bool await_ready() const {
            return test_app::frame_counter >= target_frame;
        }
        void await_suspend(std::coroutine_handle<promise_type>) {}
        void await_resume() {}
    };

    static WaitFrames wait_for_frames(int frames) { return WaitFrames{frames}; }

    template<typename Func>
    struct WaitCondition {
        Func condition;
        int max_frames;
        int start_frame;
        std::coroutine_handle<promise_type> suspended_handle;

        WaitCondition(Func cond, int max)
            : condition(cond),
              max_frames(max),
              start_frame(test_app::frame_counter),
              suspended_handle(nullptr) {}

        bool await_ready() const {
            return condition();  // Ready if condition is already met
        }

        void await_suspend(std::coroutine_handle<promise_type> coro_handle) {
            suspended_handle = coro_handle;
            // Coroutine is suspended - TestSystem will resume it when condition
            // is met or timeout
        }

        bool await_resume() {
            // Check for timeout
            if (test_app::frame_counter - start_frame >= max_frames) {
                suspended_handle = nullptr;
                throw std::runtime_error("Condition not met within max frames");
            }
            suspended_handle = nullptr;
            return condition();  // Return current condition state
        }

        // Method for TestSystem to check if ready to resume
        bool should_resume() const {
            return condition() ||
                   (test_app::frame_counter - start_frame >= max_frames);
        }

        // Method for TestSystem to resume if ready
        void resume_if_ready() {
            if (should_resume() && suspended_handle) {
                suspended_handle.resume();
            }
        }
    };

    template<typename Func>
    static WaitCondition<Func> wait_for_condition(Func condition,
                                                  int max_frames = 300) {
        return WaitCondition<Func>{condition, max_frames};
    }

    static bool has_order() {
        return afterhours::EntityQuery()
            .whereHasComponent<OrderSlot>()
            .whereHasComponent<Order>()
            .has_values();
    }

    static bool is_order_selected() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();
        return selected_order.order_id.order_id.has_value();
    }

    static bool are_all_items_selected() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return false;
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<Order>()
                 .gen()) {
            const Order &order = order_entity.get<Order>();
            return all_items_selected(order);
        }
        return false;
    }

    static bool is_order_shipped() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return false;
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<Order>()
                 .gen()) {
            const Order &order = order_entity.get<Order>();
            return order.is_shipped();
        }
        return false;
    }

    static bool is_order_fully_complete() {
        return afterhours::EntityQuery()
            .whereHasComponent<OrderSlot>()
            .whereHasComponent<Order>()
            .whereLambda([](const afterhours::Entity &entity) {
                const OrderSlot &slot = entity.get<OrderSlot>();
                const Order &order = entity.get<Order>();
                return slot.index >= 0 && order.is_fully_complete();
            })
            .has_values();
    }

    static ViewState get_current_view() {
        const afterhours::Entity &view_entity =
            afterhours::EntityHelper::get_singleton<ActiveView>();
        const ActiveView &active_view = view_entity.get<ActiveView>();
        return active_view.current_view;
    }

    static std::vector<ItemType> get_order_items() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return {};
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<Order>()
                 .gen()) {
            const Order &order = order_entity.get<Order>();
            return order.items;
        }
        return {};
    }

    static BoxingState get_boxing_state() {
        const afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::get_singleton<BoxingProgress>();
        const BoxingProgress &boxing_progress =
            boxing_progress_entity.get<BoxingProgress>();
        return boxing_progress.state;
    }

    static int get_items_placed() {
        const afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::get_singleton<BoxingProgress>();
        const BoxingProgress &boxing_progress =
            boxing_progress_entity.get<BoxingProgress>();
        return boxing_progress.items_placed;
    }

    static int get_total_items_to_box() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return 0;
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<Order>()
                 .gen()) {
            const Order &order = order_entity.get<Order>();
            return static_cast<int>(order.selected_items.size());
        }
        return 0;
    }

    static std::optional<afterhours::EntityID> get_active_order_id() {
        const afterhours::Entity &active_order_entity =
            afterhours::EntityHelper::get_singleton<ActiveOrder>();
        const ActiveOrder &active_order =
            active_order_entity.get<ActiveOrder>();
        return active_order.order_id.order_id;
    }

    static std::optional<afterhours::EntityID> get_selected_order_id() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();
        return selected_order.order_id.order_id;
    }

    static size_t get_in_progress_order_count() {
        return afterhours::EntityQuery()
            .whereHasComponent<OrderSlot>()
            .whereLambda([](const afterhours::Entity &entity) {
                const OrderSlot &slot = entity.get<OrderSlot>();
                return slot.index >= 0;
            })
            .gen_count();
    }

    static afterhours::EntityID force_create_order(
        const std::vector<ItemType> &items) {
        afterhours::Entity &order_entity =
            afterhours::EntityHelper::createEntity();
        Order &order = order_entity.addComponent<Order>();
        order.items = items;

        // Find the first available slot
        int slot_index = 0;
        std::set<int> used_slots;
        for (const OrderSlot &slot : afterhours::EntityQuery()
                                         .whereHasComponent<OrderSlot>()
                                         .gen_as<OrderSlot>()) {
            if (slot.index >= 0) {
                used_slots.insert(slot.index);
            }
        }

        while (used_slots.find(slot_index) != used_slots.end()) {
            slot_index++;
        }

        OrderSlot &slot = order_entity.addComponent<OrderSlot>();
        slot.index = slot_index;

        return order_entity.id;
    }

    static TypingStatus get_typing_status() {
        const afterhours::Entity &buffer_entity =
            afterhours::EntityHelper::get_singleton<TypingBuffer>();
        const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
        return buffer.status;
    }

    static std::string get_typing_buffer_text() {
        const afterhours::Entity &buffer_entity =
            afterhours::EntityHelper::get_singleton<TypingBuffer>();
        const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
        return buffer.buffer;
    }

    static int get_ready_stamp_progress() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();
        std::optional<afterhours::EntityID> target_id =
            selected_order.order_id.order_id;
        if (!target_id.has_value()) {
            const afterhours::Entity &active_order_entity =
                afterhours::EntityHelper::get_singleton<ActiveOrder>();
            const ActiveOrder &active_order =
                active_order_entity.get<ActiveOrder>();
            target_id = active_order.order_id.order_id;
        }
        if (!target_id.has_value()) {
            return 0;
        }
        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(target_id.value())
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
            // Derive ready stamp progress from workflow state
            switch (workflow.state) {
                case OrderState::Shipped_Stamp0:
                    return 0;
                case OrderState::Shipped_Stamp1:
                    return 1;
                case OrderState::Shipped_Stamp2:
                    return 2;
                case OrderState::Shipped_Stamp3:
                    return 3;
                default:
                    return 0;
            }
        }
        return 0;
    }

    static bool is_order_in_requesting_state() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return false;
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
            return workflow.state == OrderState::Requesting_NeedsInput;
        }
        return false;
    }

    static bool is_order_ready_for_stamping() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return false;
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
            OrderMacroState macro_state = macro_state_of(workflow.state);
            return macro_state == OrderMacroState::Shipped &&
                   (workflow.state == OrderState::Shipped_Stamp0 ||
                    workflow.state == OrderState::Shipped_Stamp1 ||
                    workflow.state == OrderState::Shipped_Stamp2 ||
                    workflow.state == OrderState::Shipped_Stamp3);
        }
        return false;
    }

    static OrderState get_selected_order_state() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return OrderState::Incoming_Arrived;  // Default
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
            return workflow.state;
        }
        return OrderState::Incoming_Arrived;  // Default
    }

    static bool is_selected_order_in_state(OrderState expected_state) {
        return get_selected_order_state() == expected_state;
    }

    static void set_selected_order_state(OrderState state) {
        afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return;
        }

        auto orders = afterhours::EntityQuery()
                          .whereID(selected_order.order_id.order_id.value())
                          .whereHasComponent<OrderWorkflow>()
                          .gen();

        for (auto &order_ref : orders) {
            afterhours::Entity &order_entity = order_ref.get();
            OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
            workflow.state = state;
            workflow.time_in_state = 0.0f;
            return;
        }
    }

    static bool is_selected_order_in_macro_state(
        OrderMacroState expected_macro) {
        OrderState current_state = get_selected_order_state();
        return macro_state_of(current_state) == expected_macro;
    }

    static std::map<ItemType, int> get_selected_order_required_counts() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        std::cout << "get_selected_order_required_counts: has selected order = "
                  << selected_order.order_id.order_id.has_value() << std::endl;

        if (!selected_order.order_id.order_id.has_value()) {
            return {};
        }

        auto order_id = selected_order.order_id.order_id.value();
        std::cout << "get_selected_order_required_counts: selected order id = "
                  << static_cast<unsigned long long>(order_id) << std::endl;

        auto entities = afterhours::EntityQuery()
                            .whereID(order_id)
                            .whereHasComponent<OrderRequiredCounts>()
                            .gen();

        std::cout << "get_selected_order_required_counts: found "
                  << entities.size() << " entities" << std::endl;

        for (const auto &entity_ref : entities) {
            const afterhours::Entity &order_entity = entity_ref.get();
            const OrderRequiredCounts &counts =
                order_entity.get<OrderRequiredCounts>();
            std::cout << "get_selected_order_required_counts: counts.size() = "
                      << counts.counts.size() << std::endl;
            return counts.counts;
        }
        std::cout << "get_selected_order_required_counts: returning empty"
                  << std::endl;
        return {};
    }

    static std::map<ItemType, int> get_selected_order_requested_counts() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return {};
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<OrderRequestedCounts>()
                 .gen()) {
            const OrderRequestedCounts &counts =
                order_entity.get<OrderRequestedCounts>();
            return counts.counts;
        }
        return {};
    }

    static std::map<ItemType, int> get_selected_order_received_counts() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();

        if (!selected_order.order_id.order_id.has_value()) {
            return {};
        }

        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(selected_order.order_id.order_id.value())
                 .whereHasComponent<OrderReceivedCounts>()
                 .gen()) {
            const OrderReceivedCounts &counts =
                order_entity.get<OrderReceivedCounts>();
            return counts.counts;
        }
        return {};
    }

    static bool are_all_items_requested() {
        auto required = get_selected_order_required_counts();
        auto requested = get_selected_order_requested_counts();

        for (const auto &[item_type, required_count] : required) {
            auto it = requested.find(item_type);
            int requested_count = (it != requested.end()) ? it->second : 0;
            if (requested_count < required_count) {
                return false;
            }
        }
        return true;
    }

    static bool are_all_items_received() {
        auto required = get_selected_order_required_counts();
        auto received = get_selected_order_received_counts();

        for (const auto &[item_type, required_count] : required) {
            auto it = received.find(item_type);
            int received_count = (it != received.end()) ? it->second : 0;
            if (received_count < required_count) {
                return false;
            }
        }
        return true;
    }

    // Ready stamp char application is now handled by ProcessReadyStampSystem
    static void apply_ready_stamp_char(char c) {
        // This function is deprecated - ready stamp input is handled by the
        // ProcessReadyStampSystem
        (void) c;  // Suppress unused parameter warning
    }

    static void force_ready_complete() {
        const afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &selected_order =
            selected_order_entity.get<SelectedOrder>();
        std::optional<afterhours::EntityID> target_id =
            selected_order.order_id.order_id;
        if (!target_id.has_value()) {
            const afterhours::Entity &active_order_entity =
                afterhours::EntityHelper::get_singleton<ActiveOrder>();
            const ActiveOrder &active_order =
                active_order_entity.get<ActiveOrder>();
            target_id = active_order.order_id.order_id;
        }
        if (!target_id.has_value()) {
            return;
        }

        for (afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(target_id.value())
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
            workflow.state = OrderState::Shipped_Stamp3;
            workflow.time_in_state = 0.0f;
            break;
        }
    }
};
