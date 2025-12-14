#include "order_state_machine.h"
#include <algorithm>

// Completion predicate implementations

bool is_incoming_arrived_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Input state - never auto-complete
  return false;
}

bool is_opened_active_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Input state - never auto-complete
  return false;
}

bool is_requesting_input_error_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - complete after short feedback period
  return time_in_state >= 0.25f; // 250ms feedback
}

bool is_requesting_all_requested_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - immediate handoff
  return time_in_state >= 0.0f;
}

bool is_receiving_on_conveyor_waiting_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - complete when conveyor item starts moving
  // TODO: Implement proper conveyor item checking
  // Consider: Query ConveyorItem entities for this order and check if any are moving
  // Currently assumes immediate completion for demo purposes
  return time_in_state >= 0.0f;
}

bool is_receiving_on_conveyor_moving_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - complete when item reaches ready area
  // TODO: Implement actual conveyor position checking - currently assumes immediate completion
  // Consider: Query conveyor items for this order and check if any have reached the ready threshold
  return true; // TODO: implement actual conveyor position checking
}

bool is_receiving_received_to_ready_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - decide whether to loop or go to ReadyToBox
  // Check if all required items have been received
  bool all_received = true;
  for (const auto& [item_type, required_count] : required_counts) {
    auto received_it = received_counts.find(item_type);
    int received_count = (received_it != received_counts.end()) ? received_it->second : 0;
    if (received_count < required_count) {
      all_received = false;
      break;
    }
  }
  return all_received;
}

bool is_ready_to_box_staged_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - immediate handoff to boxing
  return time_in_state >= 0.0f;
}

bool is_complete_closeout_delay_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  // Processing state - complete after 1 second delay
  return time_in_state >= 1.0f;
}

// State table definition - single source of truth
const std::map<OrderState, StateSpec> STATE_TABLE = {
  // Incoming
  {OrderState::Incoming_Arrived, StateSpec{
    .macro = OrderMacroState::Incoming,
    .kind = OrderStateKind::Input,
    .next = OrderState::Opened_Active,
    .min_duration_seconds = 0.0f,
    .is_complete = is_incoming_arrived_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Incoming,
      .to_macro = OrderMacroState::Opened,
      .dash_index = 0,
      .dash_count = 1
    }
  }},

  // Opened
  {OrderState::Opened_Active, StateSpec{
    .macro = OrderMacroState::Opened,
    .kind = OrderStateKind::Input,
    .next = OrderState::Requesting_NeedsInput,
    .min_duration_seconds = 0.0f,
    .is_complete = is_opened_active_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Opened,
      .to_macro = OrderMacroState::RequestingItems,
      .dash_index = 0,
      .dash_count = 1
    }
  }},

  // RequestingItems
  {OrderState::Requesting_NeedsInput, StateSpec{
    .macro = OrderMacroState::RequestingItems,
    .kind = OrderStateKind::Input,
    .next = OrderState::Requesting_AllRequested, // Will be overridden by input logic
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr, // Input states don't auto-complete
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::RequestingItems,
      .to_macro = OrderMacroState::ReceivingItems,
      .dash_index = 0,
      .dash_count = 1
    }
  }},
  {OrderState::Requesting_InputError, StateSpec{
    .macro = OrderMacroState::RequestingItems,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Requesting_NeedsInput,
    .min_duration_seconds = 0.25f,
    .is_complete = is_requesting_input_error_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::RequestingItems,
      .to_macro = OrderMacroState::ReceivingItems,
      .dash_index = 0,
      .dash_count = 1
    }
  }},
  {OrderState::Requesting_AllRequested, StateSpec{
    .macro = OrderMacroState::RequestingItems,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Receiving_OnConveyorWaiting,
    .min_duration_seconds = 0.0f,
    .is_complete = is_requesting_all_requested_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::RequestingItems,
      .to_macro = OrderMacroState::ReceivingItems,
      .dash_index = 0,
      .dash_count = 1
    }
  }},

  // ReceivingItems
  {OrderState::Receiving_OnConveyorWaiting, StateSpec{
    .macro = OrderMacroState::ReceivingItems,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Receiving_OnConveyorMoving,
    .min_duration_seconds = 0.0f,
    .is_complete = is_receiving_on_conveyor_waiting_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::ReceivingItems,
      .to_macro = OrderMacroState::ReadyToBox,
      .dash_index = 0,
      .dash_count = 3
    }
  }},
  {OrderState::Receiving_OnConveyorMoving, StateSpec{
    .macro = OrderMacroState::ReceivingItems,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Receiving_ReceivedToReady,
    .min_duration_seconds = 0.0f,
    .is_complete = is_receiving_on_conveyor_moving_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::ReceivingItems,
      .to_macro = OrderMacroState::ReadyToBox,
      .dash_index = 1,
      .dash_count = 3
    }
  }},
  {OrderState::Receiving_ReceivedToReady, StateSpec{
    .macro = OrderMacroState::ReceivingItems,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Receiving_OnConveyorWaiting, // Will be overridden by completion logic
    .min_duration_seconds = 0.0f,
    .is_complete = is_receiving_received_to_ready_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::ReceivingItems,
      .to_macro = OrderMacroState::ReadyToBox,
      .dash_index = 2,
      .dash_count = 3
    }
  }},

  // ReadyToBox
  {OrderState::ReadyToBox_Staged, StateSpec{
    .macro = OrderMacroState::ReadyToBox,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Boxing_FoldBox,
    .min_duration_seconds = 0.0f,
    .is_complete = is_ready_to_box_staged_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::ReadyToBox,
      .to_macro = OrderMacroState::Boxing,
      .dash_index = 0,
      .dash_count = 1
    }
  }},

  // Boxing (all Input states)
  {OrderState::Boxing_FoldBox, StateSpec{
    .macro = OrderMacroState::Boxing,
    .kind = OrderStateKind::Input,
    .next = OrderState::Boxing_PutItems,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Boxing,
      .to_macro = OrderMacroState::Shipped,
      .dash_index = 0,
      .dash_count = 5
    }
  }},
  {OrderState::Boxing_PutItems, StateSpec{
    .macro = OrderMacroState::Boxing,
    .kind = OrderStateKind::Input,
    .next = OrderState::Boxing_Fold,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Boxing,
      .to_macro = OrderMacroState::Shipped,
      .dash_index = 1,
      .dash_count = 5
    }
  }},
  {OrderState::Boxing_Fold, StateSpec{
    .macro = OrderMacroState::Boxing,
    .kind = OrderStateKind::Input,
    .next = OrderState::Boxing_Tape,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Boxing,
      .to_macro = OrderMacroState::Shipped,
      .dash_index = 2,
      .dash_count = 5
    }
  }},
  {OrderState::Boxing_Tape, StateSpec{
    .macro = OrderMacroState::Boxing,
    .kind = OrderStateKind::Input,
    .next = OrderState::Boxing_Ship,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Boxing,
      .to_macro = OrderMacroState::Shipped,
      .dash_index = 3,
      .dash_count = 5
    }
  }},
  {OrderState::Boxing_Ship, StateSpec{
    .macro = OrderMacroState::Boxing,
    .kind = OrderStateKind::Input,
    .next = OrderState::Shipped_Stamp0,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Boxing,
      .to_macro = OrderMacroState::Shipped,
      .dash_index = 4,
      .dash_count = 5
    }
  }},

  // Shipped (all Input states)
  {OrderState::Shipped_Stamp0, StateSpec{
    .macro = OrderMacroState::Shipped,
    .kind = OrderStateKind::Input,
    .next = OrderState::Shipped_Stamp1,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Shipped,
      .to_macro = OrderMacroState::Complete,
      .dash_index = 0,
      .dash_count = 5
    }
  }},
  {OrderState::Shipped_Stamp1, StateSpec{
    .macro = OrderMacroState::Shipped,
    .kind = OrderStateKind::Input,
    .next = OrderState::Shipped_Stamp2,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Shipped,
      .to_macro = OrderMacroState::Complete,
      .dash_index = 1,
      .dash_count = 5
    }
  }},
  {OrderState::Shipped_Stamp2, StateSpec{
    .macro = OrderMacroState::Shipped,
    .kind = OrderStateKind::Input,
    .next = OrderState::Shipped_Stamp3,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Shipped,
      .to_macro = OrderMacroState::Complete,
      .dash_index = 2,
      .dash_count = 5
    }
  }},
  {OrderState::Shipped_Stamp3, StateSpec{
    .macro = OrderMacroState::Shipped,
    .kind = OrderStateKind::Input,
    .next = OrderState::Complete_CloseoutDelay,
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Shipped,
      .to_macro = OrderMacroState::Complete,
      .dash_index = 3,
      .dash_count = 5
    }
  }},

  // Complete
  {OrderState::Complete_CloseoutDelay, StateSpec{
    .macro = OrderMacroState::Complete,
    .kind = OrderStateKind::Processing,
    .next = OrderState::Complete_ClosedOut,
    .min_duration_seconds = 1.0f,
    .is_complete = is_complete_closeout_delay_complete,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Complete,
      .to_macro = OrderMacroState::Complete,
      .dash_index = 0,
      .dash_count = 1
    }
  }},
  {OrderState::Complete_ClosedOut, StateSpec{
    .macro = OrderMacroState::Complete,
    .kind = OrderStateKind::Terminal,
    .next = OrderState::Complete_ClosedOut, // Terminal states don't transition
    .min_duration_seconds = 0.0f,
    .is_complete = nullptr,
    .timeline_segment = TimelineSegment{
      .from_macro = OrderMacroState::Complete,
      .to_macro = OrderMacroState::Complete,
      .dash_index = 0,
      .dash_count = 1
    }
  }},
};

// Helper function implementations

OrderMacroState macro_state_of(OrderState state) {
  auto it = STATE_TABLE.find(state);
  return (it != STATE_TABLE.end()) ? it->second.macro : OrderMacroState::Incoming;
}

OrderStateKind kind_of(OrderState state) {
  auto it = STATE_TABLE.find(state);
  return (it != STATE_TABLE.end()) ? it->second.kind : OrderStateKind::Terminal;
}

OrderState next_of(OrderState state) {
  auto it = STATE_TABLE.find(state);
  return (it != STATE_TABLE.end()) ? it->second.next : state;
}

bool should_flash(OrderState state, bool is_selected) {
  return is_selected && (kind_of(state) == OrderStateKind::Input);
}

TimelineSegment timeline_segment_of(OrderState state) {
  auto it = STATE_TABLE.find(state);
  return (it != STATE_TABLE.end()) ? it->second.timeline_segment : TimelineSegment{};
}

// State advancement implementation
OrderState advance_if_complete(OrderState current, float dt, float& time_in_state, const Order& order, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts) {
  auto it = STATE_TABLE.find(current);
  if (it == STATE_TABLE.end()) {
    log_error("Unknown order state: {}", static_cast<int>(current));
    return current;
  }

  const StateSpec& spec = it->second;

  // Terminal states never advance
  if (spec.kind == OrderStateKind::Terminal) {
    return current;
  }

  // Input states never auto-advance
  if (spec.kind == OrderStateKind::Input) {
    return current;
  }

  // Processing states: accumulate time and check completion
  time_in_state += dt;

  if (spec.is_complete && spec.is_complete(order, time_in_state, required_counts, received_counts)) {
    // TODO: State advancement logic needs generalization
    // Consider: Move conditional next logic into the completion predicates
    // Consider: Support more complex state transition graphs
    // Consider: Add transition validation (prevent invalid state changes)
    // For states with conditional next (like Receiving_ReceivedToReady), we need special handling
    if (current == OrderState::Receiving_ReceivedToReady) {
      // Check if all items received - if yes, go to ReadyToBox, else loop back
      bool all_received = true;
      for (const auto& [item_type, required_count] : required_counts) {
        auto received_it = received_counts.find(item_type);
        int received_count = (received_it != received_counts.end()) ? received_it->second : 0;
        if (received_count < required_count) {
          all_received = false;
          break;
        }
      }
      time_in_state = 0.0f;
      return all_received ? OrderState::ReadyToBox_Staged : OrderState::Receiving_OnConveyorWaiting;
    }

    time_in_state = 0.0f;
    return spec.next;
  }

  return current;
}

// TODO: Input-driven advancement needs proper implementation
// Consider: Replace simplified logic with state-specific input validation
// - Boxing states: Check for specific keys (P, F, T, S) in sequence
// - Requesting states: Validate typed item keys match available items
// - Stamping states: Check for correct READY/TO/SHIP sequence
// - Opening states: Allow any input to advance
bool try_advance_on_input(Order& order, OrderState& current_state, float& time_in_state, const std::string& input_event) {
  auto it = STATE_TABLE.find(current_state);
  if (it == STATE_TABLE.end() || it->second.kind != OrderStateKind::Input) {
    return false;
  }

  // This is a simplified version - actual implementation would need to check
  // if the input_event matches what's expected for this state
  // For now, assume any input in an Input state advances it
  time_in_state = 0.0f;
  current_state = it->second.next;
  return true;
}