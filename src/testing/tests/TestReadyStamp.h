#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

TEST(test_ready_stamp_sequence) {
  bool order_generated = co_await TestApp::wait_for_condition(
      []() { return TestApp::has_order(); }, 3600);
  if (!order_generated) {
    throw std::runtime_error("No order generated for ready stamp test");
  }

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 60);

  std::vector<ItemType> order_items = TestApp::get_order_items();
  for (ItemType item_type : order_items) {
    std::string item_name = item_type_to_string(item_type);
    TestApp::simulate_typing(item_name);
    co_await TestApp::wait_for_frames(3);
  }

  co_await TestApp::wait_for_condition(
      []() { return TestApp::are_all_items_selected(); }, 120);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::PutItems; },
      60);

  int total_items = TestApp::get_total_items_to_box();
  for (int i = 0; i < total_items; ++i) {
    TestApp::simulate_key(raylib::KEY_P);
    co_await TestApp::wait_for_frames(2);
  }

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::Fold; }, 60);

  TestApp::simulate_key(raylib::KEY_F);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::Tape; }, 60);

  TestApp::simulate_key(raylib::KEY_T);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::Ship; }, 60);

  TestApp::simulate_key(raylib::KEY_S);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_shipped(); }, 60);

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 120);

  // Stamp READY/TO/SHIP via input
  TestApp::simulate_key(raylib::KEY_R);
  co_await TestApp::wait_for_frames(1);
  if (TestApp::get_ready_stamp_progress() < 1) {
    throw std::runtime_error("R stamp not recorded");
  }

  TestApp::simulate_key(raylib::KEY_T);
  co_await TestApp::wait_for_frames(6);
  if (TestApp::get_ready_stamp_progress() < 2) {
    throw std::runtime_error("T stamp not recorded");
  }

  TestApp::simulate_key(raylib::KEY_S);
  co_await TestApp::wait_for_frames(6);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_fully_complete(); }, 120);

  co_return;
}
