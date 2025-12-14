#pragma once

#include <afterhours/ah.h>

#include <stdexcept>

#include "../../components.h"
#include "../../order_components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"

TEST(test_order_selection) {
    // Wait for order to be generated
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

    // Verify order is in Requesting_NeedsInput state (selection opens it
    // directly)
    if (!TestApp::is_selected_order_in_state(
            OrderState::Requesting_NeedsInput)) {
        throw std::runtime_error(
            "Selected order not in Requesting_NeedsInput state");
    }

    // Verify we can get the order item counts
    auto required_counts = TestApp::get_selected_order_required_counts();
    if (required_counts.empty()) {
        throw std::runtime_error("Selected order has no required items");
    }

    // Deselect order with ESC (should go back to Incoming_Arrived or stay in
    // Opened_Active but not selected)
    TestApp::simulate_key(raylib::KEY_ESCAPE);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return !TestApp::is_order_selected(); }, 60);

    // Select order again
    TestApp::simulate_key(raylib::KEY_ONE);
    co_await TestApp::wait_for_frames(2);

    co_await TestApp::wait_for_condition(
        []() { return TestApp::is_order_selected(); }, 60);

    // Verify state is still Requesting_NeedsInput (re-selection doesn't change
    // state)
    if (!TestApp::is_selected_order_in_state(
            OrderState::Requesting_NeedsInput)) {
        throw std::runtime_error(
            "Re-selected order not in Requesting_NeedsInput state");
    }

    co_return 0;
}
