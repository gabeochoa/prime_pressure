#pragma once

#include <afterhours/ah.h>
#include <sys/resource.h>

#include "item_type.h"
#include "log.h"
#include "std_include.h"

template<typename Component>
inline Component &get_singleton_as() {
    afterhours::Entity &entity =
        afterhours::EntityHelper::get_singleton<Component>();
    return entity.get<Component>();
}

template<typename Component>
inline Component &get_singleton_component() {
    return get_singleton_as<Component>();
}

// Include order components after ItemType is defined
#include "order_components.h"

struct Order : afterhours::BaseComponent {
    std::vector<ItemType> items;
    std::vector<ItemType> selected_items;
    std::vector<ItemType> ready_items;
    bool has_been_selected = false;
    int items_completed = 0;

    // Helper methods using the new state machine
    static int get_ready_stamp_progress(
        const afterhours::Entity &order_entity) {
        // Check OrderWorkflow state to determine stamping progress
        const OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
        switch (workflow.state) {
            // Incoming states
            case OrderState::Incoming_Arrived:
            case OrderState::Incoming_Backlogged:
            // Opened states
            case OrderState::Opened_Active:
            case OrderState::Opened_Inactive:
            // Requesting states
            case OrderState::Requesting_NeedsInput:
            case OrderState::Requesting_InputError:
            case OrderState::Requesting_AllRequested:
            // Receiving states
            case OrderState::Receiving_OnConveyorWaiting:
            case OrderState::Receiving_OnConveyorMoving:
            case OrderState::Receiving_ReceivedToReady:
            // ReadyToBox states
            case OrderState::ReadyToBox_Staged:
            case OrderState::ReadyToBox_Queued:
            // Boxing states
            case OrderState::Boxing_FoldBox:
            case OrderState::Boxing_PutItems:
            case OrderState::Boxing_Fold:
            case OrderState::Boxing_Tape:
            case OrderState::Boxing_Ship:
                return 0;  // Not stamped yet
            // Shipped states (stamping progress)
            case OrderState::Shipped_Stamp0:
                return 0;
            case OrderState::Shipped_Stamp1:
                return 1;
            case OrderState::Shipped_Stamp2:
                return 2;
            case OrderState::Shipped_Stamp3:
                return 3;
            // Complete states
            case OrderState::Complete_CloseoutDelay:
            case OrderState::Complete_ClosedOut:
                return 4;  // Fully complete
        }
        return 0;  // Fallback (should not reach here)
    }

    bool is_shipped() const {
        // Check if order is in shipped or complete states (since shipping now
        // goes directly to complete)
        for (const afterhours::Entity &entity :
             afterhours::EntityQuery()
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const OrderWorkflow &workflow = entity.get<OrderWorkflow>();
            OrderMacroState macro_state = macro_state_of(workflow.state);
            return macro_state == OrderMacroState::Shipped ||
                   macro_state == OrderMacroState::Complete;
        }
        return false;
    }

    bool is_fully_complete() const {
        // This should be replaced with logic that checks if order is in
        // Complete_ClosedOut state For now, return false as a placeholder
        return false;
    }

    bool is_ready_to_pack() const {
        // This should be replaced with logic that checks if order is in
        // ReadyToBox states For now, return false as a placeholder
        return false;
    }

    bool should_flash_conveyor() const {
        // This should be replaced with logic that checks workflow state
        // For now, return false as a placeholder
        return false;
    }
};

// Timeline helper functions

// Order-related helper functions
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

struct ShippingAnimation : afterhours::BaseComponent {
    bool is_active = false;
    float animation_time = 0.0f;
    float animation_duration = 1.0f;  // seconds
    std::optional<afterhours::EntityID> order_id;
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
    int index = -1;  // -1 means no slot assigned
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
