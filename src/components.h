#pragma once

#include "log.h"
#include "std_include.h"
#include <afterhours/ah.h>
#include <magic_enum/magic_enum.hpp>

enum struct ItemType { Book, Pen, Mug, Cup, Bag, Box, Toy, Hat, Key, Map };

struct Order : afterhours::BaseComponent {
  std::vector<ItemType> items;
  std::vector<ItemType> selected_items;
  std::vector<ItemType> ready_items;
  bool is_complete = false;
  bool is_shipped = false;
  bool is_fully_complete = false;
  bool has_been_selected = false;
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

const std::map<ItemType, char> ITEM_KEY_MAP = {
    {ItemType::Book, 'b'}, {ItemType::Pen, 'p'}, {ItemType::Mug, 'm'},
    {ItemType::Cup, 'c'},  {ItemType::Bag, 'a'}, {ItemType::Box, 'o'},
    {ItemType::Toy, 't'},  {ItemType::Hat, 'h'}, {ItemType::Key, 'k'},
    {ItemType::Map, 'x'}};

inline void validate_item_key_map() {
  const auto all_items = magic_enum::enum_values<ItemType>();
  for (ItemType item : all_items) {
    if (ITEM_KEY_MAP.find(item) == ITEM_KEY_MAP.end()) {
      log_error("ItemType {} is missing a key mapping in ITEM_KEY_MAP",
                magic_enum::enum_name(item));
    }
  }
}

namespace {
static const bool _validate_item_keys = (validate_item_key_map(), true);
}

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

inline std::string format_item_with_key(ItemType type) {
  auto it = ITEM_KEY_MAP.find(type);
  if (it == ITEM_KEY_MAP.end()) {
    return item_type_to_string(type);
  }
  char key = it->second;
  std::string name = item_type_to_string(type);
  std::string result;
  bool found_key = false;
  for (size_t i = 0; i < name.length(); ++i) {
    if (!found_key && std::tolower(static_cast<unsigned char>(name[i])) ==
                          std::tolower(static_cast<unsigned char>(key))) {
      result += "[";
      result += name[i];
      result += "]";
      found_key = true;
    } else {
      result += name[i];
    }
  }
  if (!found_key) {
    result = "[" + std::string(1, key) + "]" + name;
  }
  return result;
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

inline bool all_items_ready(const Order &order) {
  std::map<ItemType, int> item_counts = count_items(order.items);
  std::map<ItemType, int> ready_counts = count_items(order.ready_items);

  for (const auto &[item_type, needed_count] : item_counts) {
    int ready_count = ready_counts[item_type];
    if (ready_count < needed_count) {
      return false;
    }
  }
  return true;
}
