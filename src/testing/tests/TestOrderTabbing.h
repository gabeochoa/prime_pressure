#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

TEST(test_order_tabbing) {
  co_await TestApp::wait_for_frames(180);

  bool have_two = co_await TestApp::wait_for_condition(
      []() { return TestApp::get_in_progress_order_count() >= 2; }, 6000);
  if (!have_two) {
    co_return;
  }

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 60);

  std::optional<afterhours::EntityID> first_id =
      TestApp::get_selected_order_id();

  TestApp::simulate_char('b');
  co_await TestApp::wait_for_frames(2);

  if (TestApp::get_typing_buffer_text().empty()) {
    throw std::runtime_error("Typing buffer did not capture input");
  }

  TestApp::simulate_key(raylib::KEY_TAB);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      [first_id]() {
        std::optional<afterhours::EntityID> active_id =
            TestApp::get_active_order_id();
        return active_id.has_value() &&
               (!first_id.has_value() || active_id.value() != first_id.value());
      },
      120);

  std::optional<afterhours::EntityID> selected_id =
      TestApp::get_selected_order_id();
  if (!selected_id.has_value()) {
    throw std::runtime_error("No selected order after tab");
  }
  if (first_id.has_value() && selected_id.value() == first_id.value()) {
    throw std::runtime_error("TAB did not change the active order");
  }
  if (!TestApp::get_typing_buffer_text().empty()) {
    throw std::runtime_error("Typing buffer not cleared after tab");
  }

  co_return;
}
