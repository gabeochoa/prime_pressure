#pragma once

#include <afterhours/ah.h>

#include <stdexcept>

#include "../../components.h"
#include "../../order_components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"

TEST(test_complete_order) {
    // Full end-to-end workflow:
    // - wait for order
    // - select slot 1
    // - type required items (request)
    // - wait for items to arrive to ready area (receive)
    // - boxing: B, P... then F, T, S
    // - stamping: R, T, S, then S to finalize shipping animation
    // - wait for Complete_ClosedOut

    log_info("TCO: waiting for any order to exist");
    bool order_generated = co_await TestApp::wait_for_condition(
        []() { return TestApp::has_order(); }, 1200);
    if (!order_generated) {
        throw std::runtime_error("No order generated after waiting");
    }
    log_info("TCO: order exists");

    // Select first order (slot 0 => KEY_ONE)
    log_info("TCO: simulating KEY_ONE to select slot 1");
    TestApp::simulate_key(raylib::KEY_ONE);
    co_await TestApp::wait_for_frames(2);

    log_info("TCO: waiting for selected order");
    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_order_selected(); }, 120);
    log_info("TCO: selected order id = {}",
             TestApp::get_selected_order_id().has_value()
                 ? std::to_string(static_cast<unsigned long long>(
                       TestApp::get_selected_order_id().value()))
                 : std::string("<none>"));

    auto selected_state = TestApp::get_selected_order_state();
    log_info("TCO: selected order state after selection = {} ({})",
             magic_enum::enum_name(selected_state),
             static_cast<int>(selected_state));

    if (!TestApp::is_selected_order_in_state(
            OrderState::Requesting_NeedsInput)) {
        throw std::runtime_error(
            "Order not in Requesting_NeedsInput after selection");
    }
    log_info("TCO: reached Requesting_NeedsInput, beginning typing phase");

    // Type the required item keys (requests items)
    auto required_counts = TestApp::get_selected_order_required_counts();
    log_info("TCO: required_counts.size() = {}", required_counts.size());
    if (required_counts.empty()) {
        throw std::runtime_error("Selected order has no required item counts");
    }

    for (const auto &[it, cnt] : required_counts) {
        log_info("TCO: requires {} x{} (key='{}')", magic_enum::enum_name(it),
                 cnt, std::string(1, item_key_for(it)));
    }

    // STRICT: typing must cause requested-counts to increment, per key press.
    for (const auto &[item_type, required_count] : required_counts) {
        for (int i = 0; i < required_count; ++i) {
            // Ensure we're in the input state for requesting.
            co_await TestApp::wait_for_condition(
                []() {
                    return TestApp::is_selected_order_in_state(
                        OrderState::Requesting_NeedsInput);
                },
                120);

            auto before = TestApp::get_selected_order_requested_counts();
            int before_count = 0;
            if (auto it = before.find(item_type); it != before.end()) {
                before_count = it->second;
            }

            char key = item_key_for(item_type);
            log_info("TCO: typing '{}' for {} (index {}/{}, before_count={})",
                     std::string(1, key), magic_enum::enum_name(item_type),
                     i + 1, required_count, before_count);
            TestApp::simulate_char(key);
            co_await TestApp::wait_for_frames(3);

            auto after = TestApp::get_selected_order_requested_counts();
            int after_count = 0;
            if (auto it = after.find(item_type); it != after.end()) {
                after_count = it->second;
            }
            log_info("TCO: typed '{}' -> after_count={}", std::string(1, key),
                     after_count);

            if (after_count != before_count + 1) {
                throw std::runtime_error(
                    "Typing did not request expected item (key '" +
                    std::string(1, key) + "'): requested_count for " +
                    std::string(magic_enum::enum_name(item_type)) + " was " +
                    std::to_string(before_count) + " then " +
                    std::to_string(after_count));
            }
        }
    }

    co_await TestApp::wait_for_condition(
        []() { return TestApp::are_all_items_requested(); }, 600);

    // STRICT but non-flaky: states can be very transient (same-frame advances),
    // so we require they were *observed* in the per-frame trace.
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::was_selected_order_state_seen(
                OrderState::Requesting_AllRequested);
        },
        240);
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::was_selected_order_state_seen(
                OrderState::Receiving_OnConveyorWaiting);
        },
        240);
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::was_selected_order_state_seen(
                OrderState::Receiving_OnConveyorMoving);
        },
        240);
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::was_selected_order_state_seen(
                OrderState::Receiving_ReceivedToReady);
        },
        240);

    // Wait for state machine to reach ReadyToBox_Staged
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::ReadyToBox_Staged);
        },
        2000);

    // Boxing: wait for Boxing_FoldBox, then B, then P x N, then F, T, S
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_FoldBox);
        },
        240);

    // STRICT: boxing_progress must be initialized for this order.
    co_await TestApp::wait_for_condition(
        []() { return TestApp::get_boxing_state() == BoxingState::FoldBox; },
        600);

    // STRICT: we expect no backlog of synthetic input by this point.
    co_await TestApp::wait_for_condition(
        []() { return test_input::input_queue.empty(); }, 600);

    // STRICT: boxing input is processed only for the active order.
    if (TestApp::get_active_order_id() != TestApp::get_selected_order_id()) {
        throw std::runtime_error(
            "ActiveOrder does not match SelectedOrder at boxing start");
    }

    TestApp::simulate_key(raylib::KEY_B);
    co_await TestApp::wait_for_frames(2);

    // STRICT: B must be consumed (otherwise boxing input isn't being processed)
    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_input_queue_empty(); }, 120);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_PutItems);
        },
        120);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::get_boxing_state() == BoxingState::PutItems; },
        120);

    int total_items = 0;
    for (const auto &[_, count] : required_counts) {
        total_items += count;
    }

    for (int i = 0; i < total_items; ++i) {
        TestApp::simulate_key(raylib::KEY_P);
        co_await TestApp::wait_for_frames(2);
    }

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(OrderState::Boxing_Fold);
        },
        240);

    TestApp::simulate_key(raylib::KEY_F);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(OrderState::Boxing_Tape);
        },
        240);

    TestApp::simulate_key(raylib::KEY_T);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(OrderState::Boxing_Ship);
        },
        240);

    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(2);

    // Stamping: R, T, S, then S to start animation -> CloseoutDelay
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Shipped_Stamp0);
        },
        240);

    TestApp::simulate_key(raylib::KEY_R);
    co_await TestApp::wait_for_frames(1);
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Shipped_Stamp1);
        },
        120);

    TestApp::simulate_key(raylib::KEY_T);
    co_await TestApp::wait_for_frames(1);
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Shipped_Stamp2);
        },
        120);

    // Finalize shipping
    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(1);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Complete_CloseoutDelay);
        },
        240);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Complete_ClosedOut);
        },
        2400);

    co_return 0;
}
