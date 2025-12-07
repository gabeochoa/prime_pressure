#pragma once

#include "../../components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"
#include <afterhours/ah.h>
#include <stdexcept>

static char item_key_for(ItemType type) {
  switch (type) {
  case ItemType::Book:
    return 'b';
  case ItemType::Pen:
    return 'p';
  case ItemType::Mug:
    return 'm';
  case ItemType::Cup:
    return 'c';
  case ItemType::Bag:
    return 'g';
  case ItemType::Box:
    return 'x';
  case ItemType::Toy:
    return 't';
  case ItemType::Hat:
    return 'h';
  case ItemType::Key:
    return 'k';
  case ItemType::Map:
    return 'a';
  }
  return 'z';
}

TEST(test_typing_buffer_status) {
  bool order_generated = co_await TestApp::wait_for_condition(
      []() { return TestApp::has_order(); }, 3600);
  if (!order_generated) {
    throw std::runtime_error("No order generated for typing buffer test");
  }

  TestApp::simulate_key(raylib::KEY_ONE);
  co_await TestApp::wait_for_frames(2);

  co_await TestApp::wait_for_condition(
      []() { return TestApp::is_order_selected(); }, 60);

  std::vector<ItemType> order_items = TestApp::get_order_items();
  if (order_items.empty()) {
    throw std::runtime_error("Order has no items");
  }
  ItemType first_item = order_items.front();
  char match_key = item_key_for(first_item);

  TestApp::simulate_char(match_key);
  co_await TestApp::wait_for_frames(2);
  if (TestApp::get_typing_status() != TypingStatus::Match) {
    throw std::runtime_error("Typing status did not record match");
  }

  co_await TestApp::wait_for_frames(100);
  if (TestApp::get_typing_status() != TypingStatus::Idle) {
    throw std::runtime_error("Typing status did not reset after match flash");
  }

  TestApp::simulate_char('z');
  co_await TestApp::wait_for_frames(2);
  if (TestApp::get_typing_status() != TypingStatus::Error) {
    throw std::runtime_error("Typing status did not record error");
  }

  co_return;
}
