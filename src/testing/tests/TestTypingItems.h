#pragma once

#include <afterhours/ah.h>

#include <stdexcept>

#include "../../components.h"
#include "../../order_components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"

TEST(test_typing_items) {
    // Wait for order to be generated
    bool order_generated = co_await TestApp::wait_for_condition(
        []() { return TestApp::has_order(); }, 1200);

    if (!order_generated) {
        throw std::runtime_error("No order generated after waiting");
    }

    // Select first order
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

    // Get required item counts and type each required item
    auto required_counts = TestApp::get_selected_order_required_counts();

    // Type each item's key character the required number of times
    for (const auto& [item_type, required_count] : required_counts) {
        for (int i = 0; i < required_count; ++i) {
            char key = item_key_for(item_type);
            TestApp::simulate_char(key);
            // Wait for the item to be matched (MatchItemToOrder clears buffer
            // on match)
            co_await TestApp::wait_for_frames(10);
        }
    }

    // Wait for all items to be requested
    co_await TestApp::wait_for_condition(
        []() { return TestApp::are_all_items_requested(); }, 300);

    // Verify all items are requested
    if (!TestApp::are_all_items_requested()) {
        throw std::runtime_error("Not all items were requested after typing");
    }

    co_return 0;
}
