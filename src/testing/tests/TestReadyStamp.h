#pragma once

#include <afterhours/ah.h>

#include <stdexcept>

#include "../../components.h"
#include "../../order_components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"

TEST(test_ready_stamp_sequence) {
    bool order_generated = co_await TestApp::wait_for_condition(
        []() { return TestApp::has_order(); }, 3600);
    if (!order_generated) {
        throw std::runtime_error("No order generated for ready stamp test");
    }

    // Select and complete the full order workflow up to shipping
    TestApp::simulate_key(raylib::KEY_ONE);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_order_selected(); }, 60);

    // Start requesting items
    TestApp::simulate_key(raylib::KEY_ENTER);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Requesting_NeedsInput);
        },
        60);

    // Request all items
    auto required_counts = TestApp::get_selected_order_required_counts();
    for (const auto& [item_type, required_count] : required_counts) {
        for (int i = 0; i < required_count; ++i) {
            char key = item_key_for(item_type);
            TestApp::simulate_char(key);
            co_await TestApp::wait_for_frames(3);
        }
    }

    // Wait for all items to be requested and received
    co_await TestApp::wait_for_condition(
        []() { return TestApp::are_all_items_requested(); }, 120);

    // STRICT but non-flaky: require these states were *observed*.
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

    co_await TestApp::wait_for_condition(
        []() { return TestApp::are_all_items_received(); }, 300);

    // Complete boxing workflow
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_FoldBox);
        },
        60);

    // Boxing starts with FoldBox -> press B to begin putting items
    co_await TestApp::wait_for_condition(
        []() { return TestApp::get_boxing_state() == BoxingState::FoldBox; },
        600);

    TestApp::simulate_key(raylib::KEY_B);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Boxing_PutItems);
        },
        60);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::get_boxing_state() == BoxingState::PutItems; },
        120);

    int total_items = 0;
    for (const auto& [_, count] : required_counts) {
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
            return TestApp::is_selected_order_in_state(
                OrderState::Shipped_Stamp0);
        },
        60);

    // Test the READY/TO/SHIP stamp sequence
    TestApp::simulate_key(raylib::KEY_R);
    co_await TestApp::wait_for_frames(1);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Shipped_Stamp1);
        },
        60);

    TestApp::simulate_key(raylib::KEY_T);
    co_await TestApp::wait_for_frames(1);

    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Shipped_Stamp2);
        },
        60);

    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(1);

    // Stamp2 requires S to actually ship (starts animation)
    TestApp::simulate_key(raylib::KEY_S);
    co_await TestApp::wait_for_frames(1);

    // Should transition to Complete_CloseoutDelay, then Complete_ClosedOut
    co_await TestApp::wait_for_condition(
        []() {
            return TestApp::is_selected_order_in_state(
                OrderState::Complete_ClosedOut);
        },
        2000);

    co_return 0;
}
