#pragma once

#include "std_include.h"
#include <afterhours/ah.h>

enum struct ItemType { Book, Pen, Mug, Cup, Bag, Box, Toy, Hat, Key, Map };

struct Order : afterhours::BaseComponent {
  std::vector<ItemType> items;
  std::vector<ItemType> selected_items;
  bool is_complete = false;
  bool is_shipped = false;
  bool is_fully_complete = false;
  int items_completed = 0;
};

enum struct BoxingState { None, FoldBox, PutItems, Fold, Tape, Ship };

struct BoxingItemStatus : afterhours::BaseComponent {
  ItemType type;
  bool is_placed = false;
};

struct BoxingProgress : afterhours::BaseComponent {
  std::optional<afterhours::EntityID> order_id;
  BoxingState state = BoxingState::None;
  int items_placed = 0;
  std::vector<afterhours::EntityID> boxing_items;
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
  bool has_error = false;
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

struct ConveyorItem : afterhours::BaseComponent {
  ItemType type;
  float x_position = 0.0f;
  float speed = 0.3f;
  bool is_moving = false;
  int vertical_index = 0;
  afterhours::EntityID order_id;
};

inline std::map<ItemType, int> count_items(const std::vector<ItemType> &items) {
  std::map<ItemType, int> counts;
  for (ItemType item_type : items) {
    counts[item_type]++;
  }
  return counts;
}

inline bool all_items_selected(const Order &order) {
  std::map<ItemType, int> item_counts = count_items(order.items);
  std::map<ItemType, int> selected_counts = count_items(order.selected_items);

  for (const auto &[item_type, needed_count] : item_counts) {
    int selected_count = selected_counts[item_type];
    if (selected_count < needed_count) {
      return false;
    }
  }
  return true;
}
