#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

TEST(test_view_switching) {
  // Wait for order to be generated
  bool order_generated = co_await TestApp::wait_for_condition(
      []() { return TestApp::has_order(); }, 1200);

  if (!order_generated) {
    throw std::runtime_error("No order generated after waiting");
  }

  // Start on Computer view
  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Computer; }, 60);

  // Test TAB forward: Computer -> Warehouse
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Warehouse; }, 60);

  // Test TAB forward: Warehouse -> Boxing
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Boxing; }, 60);

  // Test TAB forward: Boxing -> Computer (wraps around)
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Computer; }, 60);

  // Test Shift+TAB backward: Computer -> Boxing
  TestApp::simulate_key(raylib::KEY_LEFT_SHIFT);
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Boxing; }, 60);

  // Test Shift+TAB backward: Boxing -> Warehouse
  TestApp::simulate_key(raylib::KEY_LEFT_SHIFT);
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Warehouse; }, 60);

  // Test Shift+TAB backward: Warehouse -> Computer
  TestApp::simulate_key(raylib::KEY_LEFT_SHIFT);
  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_current_view() == ViewState::Computer; }, 60);

  co_return;
}
