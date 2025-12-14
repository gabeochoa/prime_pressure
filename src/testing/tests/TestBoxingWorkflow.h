#pragma once

#include <afterhours/ah.h>

#include <stdexcept>

#include "../../components.h"
#include "../../order_components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"

TEST(test_boxing_workflow) {
    // Wait for order to be generated and arrive
    bool order_generated = co_await TestApp::wait_for_condition(
        []() { return TestApp::has_order(); }, 1200);

    if (!order_generated) {
        throw std::runtime_error("No order generated after waiting");
    }

    // Select first order (should transition from Incoming_Arrived to
    // Opened_Active)
    TestApp::simulate_key(raylib::KEY_ONE);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_order_selected(); }, 60);

    // Verify we're in Requesting_NeedsInput state (selection opens it directly)
    if (!TestApp::is_selected_order_in_state(
            OrderState::Requesting_NeedsInput)) {
        throw std::runtime_error(
            "Order not in Requesting_NeedsInput state after selection");
    }

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Requesting_NeedsInput);
        },
        60);

    // Get required item counts and type the keys for each required item
    auto required_counts = TestApp::get_selected_order_required_counts();
    for (const auto& [item_type, required_count] : required_counts) {
        for (int i = 0; i < required_count; ++i) {
            char key = item_key_for(item_type);
            TestApp::simulate_char(key);
            co_await TestApp::wait_for_frames(3);
        }
    }

    // Wait for all items to be requested (should transition to
    // Requesting_AllRequested, then through receiving states)
    co_await TestApp::wait_for_condition(
        []() { return TestApp::are_all_items_requested(); }, 120);

    // Wait for order to reach ReadyToBox_Staged (after all items are received)
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::ReadyToBox_Staged);
        },
        300);

    // Should now be in boxing state - start with FoldBox
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_macro_state(
                OrderMacroState::Boxing);
        },
        60);

    // Verify boxing sequence
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_FoldBox);
        },
        60);

    // Boxing starts with FoldBox -> press B to begin putting items
    TestApp::simulate_key(raylib::KEY_B);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_PutItems);
        },
        60);

    // Put all required items into the box
    auto item_counts = TestApp::get_selected_order_required_counts();
    int total_items = 0;
    for (const auto& [_, count] : item_counts) {
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
        60);

    TestApp::simulate_key(raylib::KEY_F);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(OrderState::Boxing_Tape);
        },
        60);

    TestApp::simulate_key(raylib::KEY_T);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(OrderState::Boxing_Ship);
        },
        60);

    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(2);

    // Should now be in shipped state ready for stamping
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_macro_state(
                OrderMacroState::Shipped);
        },
        60);

    co_return 0;
}
