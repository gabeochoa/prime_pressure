#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

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

  // Switch to Warehouse view
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Warehouse; }, 60);

  // Get order items
  std::vector<ItemType> order_items = TestApp::get_order_items();

  // Type each item with delays between characters to ensure proper matching
  for (ItemType item_type : order_items) {
    std::string item_name = item_type_to_string(item_type);
    // Type each character with a small delay
    for (char c : item_name) {
      TestApp::simulate_char(c);
      co_await TestApp::wait_for_frames(2);
    }
    // Wait for the item to be matched (MatchItemToOrder clears buffer on match)
    co_await TestApp::wait_for_frames(10);
  }

  // Wait for all items to be selected (give it plenty of time)
  co_await TestApp::wait_for_condition(
      []() { return TestApp::are_all_items_selected(); }, 300);

  // Verify all items are selected
  if (!TestApp::are_all_items_selected()) {
    throw std::runtime_error("Not all items were selected after typing");
  }

  co_return;
}

