#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

TEST(complete_order) {
  co_await TestApp::wait_for_frames(5);

  co_await TestApp::wait_for_condition([]() { return TestApp::has_order(); },
                                       600);

  if (!TestApp::has_order()) {
    throw std::runtime_error("No order generated after waiting");
  }

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 60);

  if (TestApp::get_current_view() != ViewState::Computer) {
    throw std::runtime_error("Expected to be on Computer view");
  }

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

  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Boxing; }, 60);

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::FoldBox; }, 60);

  TestApp::simulate_key(raylib::KEY_B);
  co_await TestApp::wait_for_frames(2);

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

  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Computer; }, 60);

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_fully_complete(); }, 60);

  co_return;
}
