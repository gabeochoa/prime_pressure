#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

TEST(test_order_selection) {
  // Wait for order to be generated
  bool order_generated = co_await TestApp::wait_for_condition(
      []() { return TestApp::has_order(); }, 1200);

  if (!order_generated) {
    throw std::runtime_error("No order generated after waiting");
  }

  // Ensure we're on Computer view
  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Computer; }, 60);

  // Select first order (key 1)
  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 60);

  // Verify we can get the order items
  std::vector<ItemType> items = TestApp::get_order_items();
  if (items.empty()) {
    throw std::runtime_error("Selected order has no items");
  }

  // Deselect order with ESC
  TestApp::simulate_key(raylib::KEY_ESCAPE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return !TestApp::is_order_selected(); }, 60);

  // Select order again (key 1)
  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 60);

  co_return;
}
