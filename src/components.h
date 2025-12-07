#pragma once

#include "log.h"
#include "std_include.h"
#include <afterhours/ah.h>
#include <magic_enum/magic_enum.hpp>
#include <sys/resource.h>

template <typename Component> inline Component &get_singleton_as() {
  afterhours::Entity &entity =
      afterhours::EntityHelper::get_singleton<Component>();
  return entity.get<Component>();
}

template <typename Component> inline Component &get_singleton_component() {
  return get_singleton_as<Component>();
}

enum struct ItemType { Book, Pen, Mug, Cup, Bag, Box, Toy, Hat, Key, Map };

enum struct TimelineStage { Conveyor = 0, Boxing = 1, Ready = 2, Ship = 3 };

enum class TimelinePhase { Pending, Active, Done };

enum struct TimelineStageState {
  ConveyorPending = 10,
  BoxingPending = 20,
  ReadyPending = 30,
  ShipPending = 40,
  //
  ConveyorActive = 11,
  ConveyorActiveFlash = 12,
  BoxingActive = 21,
  ReadyActive = 31,

  ReadyStamp0 = 32,
  ReadyStamp1 = 33,
  ReadyStamp2 = 34,
  ReadyStamp3 = 35,
  //
  ConveyorDone = 13,
  BoxingDone = 22,
  ReadyDone = 36,
  //
  ShipDone = 41
};

inline TimelinePhase timeline_phase(TimelineStageState state) {
  switch (state) {
  case TimelineStageState::ConveyorActiveFlash:
  case TimelineStageState::ConveyorActive:
  case TimelineStageState::BoxingActive:
  case TimelineStageState::ReadyActive:
  case TimelineStageState::ReadyStamp0:
  case TimelineStageState::ReadyStamp1:
  case TimelineStageState::ReadyStamp2:
  case TimelineStageState::ReadyStamp3:
    return TimelinePhase::Active;
  case TimelineStageState::ConveyorDone:
  case TimelineStageState::BoxingDone:
  case TimelineStageState::ReadyDone:
  case TimelineStageState::ShipDone:
    return TimelinePhase::Done;
  case TimelineStageState::ConveyorPending:
  case TimelineStageState::BoxingPending:
  case TimelineStageState::ReadyPending:
  case TimelineStageState::ShipPending:
  default:
    return TimelinePhase::Pending;
  }
}

inline TimelineStage timeline_stage_of(TimelineStageState state) {
  if (state >= TimelineStageState::ConveyorPending &&
      state <= TimelineStageState::ConveyorDone) {
    return TimelineStage::Conveyor;
  } else if (state >= TimelineStageState::BoxingPending &&
             state <= TimelineStageState::BoxingDone) {
    return TimelineStage::Boxing;
  } else if (state >= TimelineStageState::ReadyPending &&
             state <= TimelineStageState::ReadyDone) {
    return TimelineStage::Ready;
  } else {
    return TimelineStage::Ship;
  }
}

struct TimelineState {
  TimelineStageState state = TimelineStageState::ConveyorPending;
};

struct Order : afterhours::BaseComponent {
  std::vector<ItemType> items;
  std::vector<ItemType> selected_items;
  std::vector<ItemType> ready_items;
  bool has_been_selected = false;
  int items_completed = 0;
  TimelineState timeline{};

  static TimelineStageState ready_stamp_state_from_progress(int value) {
    int clamped = std::max(0, std::min(3, value));
    switch (clamped) {
    case 0:
      return TimelineStageState::ReadyStamp0;
    case 1:
      return TimelineStageState::ReadyStamp1;
    case 2:
      return TimelineStageState::ReadyStamp2;
    case 3:
    default:
      return TimelineStageState::ReadyStamp3;
    }
  }

  int get_ready_stamp_progress() const {
    switch (timeline.state) {
    case TimelineStageState::ReadyStamp0:
      return 0;
    case TimelineStageState::ReadyStamp1:
      return 1;
    case TimelineStageState::ReadyStamp2:
      return 2;
    case TimelineStageState::ReadyStamp3:
      return 3;
    case TimelineStageState::ConveyorPending:
    case TimelineStageState::BoxingPending:
    case TimelineStageState::ReadyPending:
    case TimelineStageState::ShipPending:
    case TimelineStageState::ConveyorActive:
    case TimelineStageState::ConveyorActiveFlash:
    case TimelineStageState::BoxingActive:
    case TimelineStageState::ReadyActive:
    case TimelineStageState::ConveyorDone:
    case TimelineStageState::BoxingDone:
    case TimelineStageState::ReadyDone:
    case TimelineStageState::ShipDone:
    default:
      return 0;
    }
  }

  void set_ready_stamp_progress(int value) {
    int clamped = std::max(0, std::min(3, value));
    timeline.state = ready_stamp_state_from_progress(clamped);
  }

  bool is_ready_stamp_state() const {
    switch (timeline.state) {
    case TimelineStageState::ReadyStamp0:
    case TimelineStageState::ReadyStamp1:
    case TimelineStageState::ReadyStamp2:
    case TimelineStageState::ReadyStamp3:
    case TimelineStageState::ShipDone:
      return true;
    case TimelineStageState::ConveyorPending:
    case TimelineStageState::BoxingPending:
    case TimelineStageState::ReadyPending:
    case TimelineStageState::ShipPending:
    case TimelineStageState::ConveyorActive:
    case TimelineStageState::ConveyorActiveFlash:
    case TimelineStageState::BoxingActive:
    case TimelineStageState::ReadyActive:
    case TimelineStageState::ConveyorDone:
    case TimelineStageState::BoxingDone:
    case TimelineStageState::ReadyDone:
    default:
      return false;
    }
  }

  bool is_shipped() const { return is_ready_stamp_state(); }

  bool is_fully_complete() const {
    return timeline.state == TimelineStageState::ReadyStamp3 ||
           timeline.state == TimelineStageState::ShipDone;
  }

  bool is_ready_to_pack() const {
    return timeline.state == TimelineStageState::BoxingActive;
  }

  bool should_flash_conveyor() const {
    return timeline.state == TimelineStageState::ConveyorActiveFlash;
  }
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
    {ItemType::Cup, 'c'},  {ItemType::Bag, 'g'}, {ItemType::Box, 'x'},
    {ItemType::Toy, 't'},  {ItemType::Hat, 'h'}, {ItemType::Key, 'k'},
    {ItemType::Map, 'a'}};

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
  IsInProgressOrder = 4,
  IsOnConveyor = 5,
  IsSelectedOrder = 6
};

enum struct TypingStatus { Idle, Typing, Match, Error };

struct TypingBuffer : afterhours::BaseComponent {
  std::string buffer;
  float last_input_time = 0.0f;
  float status_time = 0.0f;
  TypingStatus status = TypingStatus::Idle;
};

struct Box : afterhours::BaseComponent {
  std::vector<afterhours::EntityID> items;
  int capacity = 10;
};

struct OrderSlot : afterhours::BaseComponent {
  int index = -1; // -1 means no slot assigned
};

enum struct ViewState { Computer, Warehouse, Boxing, Cutscene };

struct ActiveView : afterhours::BaseComponent {
  ViewState current_view = ViewState::Computer;
  ViewState return_view = ViewState::Computer;
};

struct OptionalOrderID {
  std::optional<afterhours::EntityID> order_id;

  void reset_if_matching_order(afterhours::EntityID target_order_id) {
    if (this->order_id.has_value() &&
        this->order_id.value() == target_order_id) {
      this->order_id.reset();
    }
  }

  void set_order_id(afterhours::EntityID target_order_id) {
    this->order_id = target_order_id;
  }

  bool is_matching_order(afterhours::EntityID target_order_id) const {
    return this->order_id.has_value() &&
           this->order_id.value() == target_order_id;
  }
};

struct SelectedOrder : afterhours::BaseComponent {
  OptionalOrderID order_id;
};

struct ActiveOrder : afterhours::BaseComponent {
  OptionalOrderID order_id;
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
