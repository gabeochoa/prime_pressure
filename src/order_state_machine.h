#pragma once

#include "order_components.h"
#include "log.h"
#include <afterhours/ah.h>
#include <map>


// State specification containing all metadata for each microstate
struct StateSpec {
  OrderMacroState macro;
  OrderStateKind kind;
  OrderState next; // for simple states
  float min_duration_seconds; // 0 if not time-based
  bool (*is_complete)(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
  TimelineSegment timeline_segment;
};


// Forward declarations for completion predicates
bool is_incoming_arrived_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_opened_active_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_requesting_input_error_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_requesting_all_requested_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_receiving_on_conveyor_waiting_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_receiving_on_conveyor_moving_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_receiving_received_to_ready_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_ready_to_box_staged_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);
bool is_complete_closeout_delay_complete(const Order& order, float time_in_state, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);

// State table - single source of truth for all state transitions and metadata
extern const std::map<OrderState, StateSpec> STATE_TABLE;

// Helper functions for state management
OrderMacroState macro_state_of(OrderState state);
OrderStateKind kind_of(OrderState state);
OrderState next_of(OrderState state);
bool should_flash(OrderState state, bool is_selected);
TimelineSegment timeline_segment_of(OrderState state);

// State advancement functions
OrderState advance_if_complete(OrderState current, float dt, float& time_in_state, const Order& order, const std::map<ItemType, int>& required_counts, const std::map<ItemType, int>& received_counts);

// Input-driven advancement (only for Input states)
bool try_advance_on_input(Order& order, OrderState& current_state, float& time_in_state, const std::string& input_event);