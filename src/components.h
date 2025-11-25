#pragma once

#include "std_include.h"
#include <afterhours/ah.h>

enum struct ItemType { Book, Pen, Mug, Cup, Bag, Box, Toy, Hat, Key, Map };

struct Order : afterhours::BaseComponent {
  std::vector<ItemType> items;
  std::vector<ItemType> selected_items;
  bool is_complete = false;
  int items_completed = 0;
};

inline std::string item_type_to_string(ItemType type) {
  switch (type) {
  case ItemType::Book:
    return "book";
  case ItemType::Pen:
    return "pen";
  case ItemType::Mug:
    return "mug";
  case ItemType::Cup:
    return "cup";
  case ItemType::Bag:
    return "bag";
  case ItemType::Box:
    return "box";
  case ItemType::Toy:
    return "toy";
  case ItemType::Hat:
    return "hat";
  case ItemType::Key:
    return "key";
  case ItemType::Map:
    return "map";
  }
  return "unknown";
}

struct Item : afterhours::BaseComponent {
  ItemType type;

  std::string name() const { return item_type_to_string(type); }
};

enum struct GameTag : afterhours::TagId {
  IsOnShelf = 0,
  IsGrabbed = 1,
  IsBoxed = 2,
  IsBox = 3,
  IsActiveOrder = 4
};

struct TypingBuffer : afterhours::BaseComponent {
  std::string buffer;
  float last_input_time = 0.0f;
};

struct Box : afterhours::BaseComponent {
  std::vector<afterhours::EntityID> items;
  int capacity = 10;
};

struct OrderQueue : afterhours::BaseComponent {
  std::vector<afterhours::EntityID> pending_orders;
  std::vector<afterhours::EntityID> active_orders;
  int max_active_orders = 3;
};

enum struct ViewState { Computer, Warehouse, Boxing };

struct ActiveView : afterhours::BaseComponent {
  ViewState current_view = ViewState::Computer;
};

struct SelectedOrder : afterhours::BaseComponent {
  std::optional<afterhours::EntityID> order_id;
};
