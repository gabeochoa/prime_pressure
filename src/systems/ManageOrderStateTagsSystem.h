#pragma once

#include "../components.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include <afterhours/ah.h>

// Maps OrderState enum values to OrderMicroTag values for microstates
inline OrderMicroTag microstate_tag_of(OrderState state) {
  switch (state) {
    case OrderState::Incoming_Arrived: return OrderMicroTag::Incoming_Arrived;
    case OrderState::Incoming_Backlogged: return OrderMicroTag::Incoming_Backlogged;
    case OrderState::Opened_Active: return OrderMicroTag::Opened_Active;
    case OrderState::Opened_Inactive: return OrderMicroTag::Opened_Inactive;
    case OrderState::Requesting_NeedsInput: return OrderMicroTag::Requesting_NeedsInput;
    case OrderState::Requesting_InputError: return OrderMicroTag::Requesting_InputError;
    case OrderState::Requesting_AllRequested: return OrderMicroTag::Requesting_AllRequested;
    case OrderState::Receiving_OnConveyorWaiting: return OrderMicroTag::Receiving_OnConveyorWaiting;
    case OrderState::Receiving_OnConveyorMoving: return OrderMicroTag::Receiving_OnConveyorMoving;
    case OrderState::Receiving_ReceivedToReady: return OrderMicroTag::Receiving_ReceivedToReady;
    case OrderState::ReadyToBox_Staged: return OrderMicroTag::ReadyToBox_Staged;
    case OrderState::ReadyToBox_Queued: return OrderMicroTag::ReadyToBox_Queued;
    case OrderState::Boxing_FoldBox: return OrderMicroTag::Boxing_FoldBox;
    case OrderState::Boxing_PutItems: return OrderMicroTag::Boxing_PutItems;
    case OrderState::Boxing_Fold: return OrderMicroTag::Boxing_Fold;
    case OrderState::Boxing_Tape: return OrderMicroTag::Boxing_Tape;
    case OrderState::Boxing_Ship: return OrderMicroTag::Boxing_Ship;
    case OrderState::Shipped_Stamp0: return OrderMicroTag::Shipped_Stamp0;
    case OrderState::Shipped_Stamp1: return OrderMicroTag::Shipped_Stamp1;
    case OrderState::Shipped_Stamp2: return OrderMicroTag::Shipped_Stamp2;
    case OrderState::Shipped_Stamp3: return OrderMicroTag::Shipped_Stamp3;
    case OrderState::Complete_CloseoutDelay: return OrderMicroTag::Complete_CloseoutDelay;
    case OrderState::Complete_ClosedOut: return OrderMicroTag::Complete_ClosedOut;
    default:
      log_error("Unknown OrderState in microstate_tag_of: {}", static_cast<int>(state));
      return OrderMicroTag::Incoming_Arrived;
  }
}

// Maps OrderMacroState enum values to OrderMacroTag values for macrostates
inline OrderMacroTag macrostate_tag_of(OrderMacroState state) {
  switch (state) {
    case OrderMacroState::Incoming: return OrderMacroTag::Incoming;
    case OrderMacroState::Opened: return OrderMacroTag::Opened;
    case OrderMacroState::RequestingItems: return OrderMacroTag::RequestingItems;
    case OrderMacroState::ReceivingItems: return OrderMacroTag::ReceivingItems;
    case OrderMacroState::ReadyToBox: return OrderMacroTag::ReadyToBox;
    case OrderMacroState::Boxing: return OrderMacroTag::Boxing;
    case OrderMacroState::Shipped: return OrderMacroTag::Shipped;
    case OrderMacroState::Complete: return OrderMacroTag::Complete;
    default:
      log_error("Unknown OrderMacroState in macrostate_tag_of: {}", static_cast<int>(state));
      return OrderMacroTag::Incoming;
  }
}

struct ManageOrderStateTagsSystem : afterhours::System<OrderWorkflow> {
  void for_each_with(afterhours::Entity &entity, OrderWorkflow &workflow, float) override {
    // Validation: Ensure workflow state is valid
    if (STATE_TABLE.find(workflow.state) == STATE_TABLE.end()) {
      log_error("Invalid order state {} for entity {}", static_cast<int>(workflow.state),
                static_cast<unsigned long long>(entity.id));
      return;
    }

    // TODO - add a way to disable bunch of tags at once

    // Remove any existing order state tags
    // Microstate tags
    for (OrderMicroTag tag : {
      OrderMicroTag::Incoming_Arrived,
      OrderMicroTag::Incoming_Backlogged,
      OrderMicroTag::Opened_Active,
      OrderMicroTag::Opened_Inactive,
      OrderMicroTag::Requesting_NeedsInput,
      OrderMicroTag::Requesting_InputError,
      OrderMicroTag::Requesting_AllRequested,
      OrderMicroTag::Receiving_OnConveyorWaiting,
      OrderMicroTag::Receiving_OnConveyorMoving,
      OrderMicroTag::Receiving_ReceivedToReady,
      OrderMicroTag::ReadyToBox_Staged,
      OrderMicroTag::ReadyToBox_Queued,
      OrderMicroTag::Boxing_FoldBox,
      OrderMicroTag::Boxing_PutItems,
      OrderMicroTag::Boxing_Fold,
      OrderMicroTag::Boxing_Tape,
      OrderMicroTag::Boxing_Ship,
      OrderMicroTag::Shipped_Stamp0,
      OrderMicroTag::Shipped_Stamp1,
      OrderMicroTag::Shipped_Stamp2,
      OrderMicroTag::Shipped_Stamp3,
      OrderMicroTag::Complete_CloseoutDelay,
      OrderMicroTag::Complete_ClosedOut
    }) {
      entity.disableTag(tag);
    }

    // Macrostate tags
    for (OrderMacroTag tag : {
      OrderMacroTag::Incoming,
      OrderMacroTag::Opened,
      OrderMacroTag::RequestingItems,
      OrderMacroTag::ReceivingItems,
      OrderMacroTag::ReadyToBox,
      OrderMacroTag::Boxing,
      OrderMacroTag::Shipped,
      OrderMacroTag::Complete
    }) {
      entity.disableTag(tag);
    }

    // Add the correct microstate and macrostate tags
    OrderMicroTag micro_tag = microstate_tag_of(workflow.state);
    OrderMacroTag macro_tag = macrostate_tag_of(macro_state_of(workflow.state));

    entity.enableTag(micro_tag);
    entity.enableTag(macro_tag);

    // TODO: Validation system could be more comprehensive
    // Consider: Add forward-only progression checks (prevent state regression)
    // Consider: Add timing validation (ensure states don't get stuck)
    // Consider: Add deadlock detection for processing states
    // Consider: Add performance monitoring for state transitions
    // Validation: Ensure exactly one microstate tag and one macrostate tag are enabled
    int micro_tag_count = 0;
    int macro_tag_count = 0;

    // Check microstate tags
    for (OrderMicroTag check_tag : {
      OrderMicroTag::Incoming_Arrived, OrderMicroTag::Incoming_Backlogged,
      OrderMicroTag::Opened_Active, OrderMicroTag::Opened_Inactive,
      OrderMicroTag::Requesting_NeedsInput, OrderMicroTag::Requesting_InputError,
      OrderMicroTag::Requesting_AllRequested, OrderMicroTag::Receiving_OnConveyorWaiting,
      OrderMicroTag::Receiving_OnConveyorMoving, OrderMicroTag::Receiving_ReceivedToReady,
      OrderMicroTag::ReadyToBox_Staged, OrderMicroTag::ReadyToBox_Queued,
      OrderMicroTag::Boxing_FoldBox, OrderMicroTag::Boxing_PutItems,
      OrderMicroTag::Boxing_Fold, OrderMicroTag::Boxing_Tape,
      OrderMicroTag::Boxing_Ship, OrderMicroTag::Shipped_Stamp0,
      OrderMicroTag::Shipped_Stamp1, OrderMicroTag::Shipped_Stamp2,
      OrderMicroTag::Shipped_Stamp3, OrderMicroTag::Complete_CloseoutDelay,
      OrderMicroTag::Complete_ClosedOut
    }) {
      if (entity.hasTag(check_tag)) micro_tag_count++;
    }

    // Check macrostate tags
    for (OrderMacroTag check_tag : {
      OrderMacroTag::Incoming, OrderMacroTag::Opened,
      OrderMacroTag::RequestingItems, OrderMacroTag::ReceivingItems,
      OrderMacroTag::ReadyToBox, OrderMacroTag::Boxing,
      OrderMacroTag::Shipped, OrderMacroTag::Complete
    }) {
      if (entity.hasTag(check_tag)) macro_tag_count++;
    }

    if (micro_tag_count != 1 || macro_tag_count != 1) {
      log_error("Order {} has invalid tag counts: micro={}, macro={} (should be 1 each)",
                static_cast<unsigned long long>(entity.id), micro_tag_count, macro_tag_count);
    }
  }
};
