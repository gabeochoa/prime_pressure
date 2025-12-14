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

    bool order_generated = co_await TestApp::wait_for_condition(
        []() { return TestApp::has_order(); }, 1200);
    if (!order_generated) {
        throw std::runtime_error("No order generated after waiting");
    }

    // Select first order (slot 0 => KEY_ONE)
    TestApp::simulate_key(raylib::KEY_ONE);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_order_selected(); }, 120);

    if (!TestApp::is_selected_order_in_state(OrderState::Requesting_NeedsInput)) {
        throw std::runtime_error(
            "Order not in Requesting_NeedsInput after selection");
    }

    // Type the required item keys (requests items)
    auto required_counts = TestApp::get_selected_order_required_counts();
    if (required_counts.empty()) {
        throw std::runtime_error("Selected order has no required item counts");
    }

    for (const auto &[item_type, required_count] : required_counts) {
        for (int i = 0; i < required_count; ++i) {
            char key = item_key_for(item_type);
            TestApp::simulate_char(key);
            co_await TestApp::wait_for_frames(2);
        }
    }

    co_await TestApp::wait_for_condition(
        []() { return TestApp::are_all_items_requested(); }, 600);

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

    TestApp::simulate_key(raylib::KEY_B);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_PutItems);
        },
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
        []() { return TestApp::is_selected_order_in_state(OrderState::Boxing_Fold); },
        240);

    TestApp::simulate_key(raylib::KEY_F);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Boxing_Tape); },
        240);

    TestApp::simulate_key(raylib::KEY_T);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Boxing_Ship); },
        240);

    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(2);

    // Stamping: R, T, S, then S to start animation -> CloseoutDelay
    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Shipped_Stamp0); },
        240);

    TestApp::simulate_key(raylib::KEY_R);
    co_await TestApp::wait_for_frames(1);
    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Shipped_Stamp1); },
        120);

    TestApp::simulate_key(raylib::KEY_T);
    co_await TestApp::wait_for_frames(1);
    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Shipped_Stamp2); },
        120);

    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(1);
    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Shipped_Stamp3); },
        120);

    // Finalize shipping
    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(1);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Complete_CloseoutDelay); },
        240);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_selected_order_in_state(OrderState::Complete_ClosedOut); },
        2400);

    co_return 0;
}
