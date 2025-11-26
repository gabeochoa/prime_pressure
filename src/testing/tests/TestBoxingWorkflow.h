#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

TEST(test_boxing_workflow) {
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

  // Switch to Warehouse and type all items
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Warehouse; }, 60);

  std::vector<ItemType> order_items = TestApp::get_order_items();
  for (ItemType item_type : order_items) {
    std::string item_name = item_type_to_string(item_type);
    TestApp::simulate_typing(item_name);
    co_await TestApp::wait_for_frames(3);
  }

  co_await TestApp::wait_for_condition(
      []() { return TestApp::are_all_items_selected(); }, 120);

  // Switch to Boxing view
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Boxing; }, 60);

  // Select order for boxing
  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  // Test boxing workflow states
  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::FoldBox; }, 60);

  // Fold box (B key)
  TestApp::simulate_key(raylib::KEY_B);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::PutItems; }, 60);

  // Place items (P key for each item)
  int total_items = TestApp::get_total_items_to_box();
  for (int i = 0; i < total_items; ++i) {
    TestApp::simulate_key(raylib::KEY_P);
    co_await TestApp::wait_for_frames(2);
  }

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::Fold; }, 60);

  // Fold box (F key)
  TestApp::simulate_key(raylib::KEY_F);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::Tape; }, 60);

  // Tape box (T key)
  TestApp::simulate_key(raylib::KEY_T);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::Ship; }, 60);

  // Ship box (S key)
  TestApp::simulate_key(raylib::KEY_S);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_shipped(); }, 60);

  co_return;
}

