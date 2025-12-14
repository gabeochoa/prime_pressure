#pragma once

#include "item_type.h"
#include "log.h"
#include "std_include.h"
#include <afterhours/ah.h>

// Core state machine enums as defined in the specification
enum class OrderMacroState {
  Incoming,
  Opened,
  RequestingItems,
  ReceivingItems,
  ReadyToBox,
  Boxing,
  Shipped,
  Complete,
};

enum class OrderState {
  // Incoming
  Incoming_Arrived,
  Incoming_Backlogged,

  // Opened
  Opened_Active,
  Opened_Inactive,

  // RequestingItems
  Requesting_NeedsInput,
  Requesting_InputError,
  Requesting_AllRequested,

  // ReceivingItems (processing/dash steps; loops until AllReceived)
  Receiving_OnConveyorWaiting,
  Receiving_OnConveyorMoving,
  Receiving_ReceivedToReady,

  // ReadyToBox
  ReadyToBox_Staged,
  ReadyToBox_Queued,

  // Boxing
  Boxing_FoldBox,
  Boxing_PutItems,
  Boxing_Fold,
  Boxing_Tape,
  Boxing_Ship,

  // Shipped (READY/TO/SHIP confirmation)
  Shipped_Stamp0,
  Shipped_Stamp1,
  Shipped_Stamp2,
  Shipped_Stamp3,

  // Complete
  Complete_CloseoutDelay,
  Complete_ClosedOut,
};

enum class OrderStateKind {
  Input,
  Processing,
  Terminal,
};

// Timeline metadata for rendering dashes between macro milestones
struct TimelineSegment {
  OrderMacroState from_macro;
  OrderMacroState to_macro;
  int dash_index = 0; // position within the segment (0-based)
  int dash_count = 1; // total dashes in this segment
};

// Order-related enums and types

// General game tags
enum struct GeneralTag : afterhours::TagId {
  IsOnShelf = 0,
  IsGrabbed = 1,
  IsBoxed = 2,
  IsBox = 3,
  IsInProgressOrder = 4,
  IsOnConveyor = 5,
  IsSelectedOrder = 6,
};

// Order macro state tags (high-level order phases)
enum struct OrderMacroTag : afterhours::TagId {
  Incoming = 10,
  Opened = 11,
  RequestingItems = 12,
  ReceivingItems = 13,
  ReadyToBox = 14,
  Boxing = 15,
  Shipped = 16,
  Complete = 17,
};

// Order micro state tags (detailed state machine states)
enum struct OrderMicroTag : afterhours::TagId {
  Incoming_Arrived = 20,
  Incoming_Backlogged = 21,
  Opened_Active = 22,
  Opened_Inactive = 23,
  Requesting_NeedsInput = 24,
  Requesting_InputError = 25,
  Requesting_AllRequested = 26,
  Receiving_OnConveyorWaiting = 27,
  Receiving_OnConveyorMoving = 28,
  Receiving_ReceivedToReady = 29,
  ReadyToBox_Staged = 30,
  ReadyToBox_Queued = 31,
  Boxing_FoldBox = 32,
  Boxing_PutItems = 33,
  Boxing_Fold = 34,
  Boxing_Tape = 35,
  Boxing_Ship = 36,
  Shipped_Stamp0 = 37,
  Shipped_Stamp1 = 38,
  Shipped_Stamp2 = 39,
  Shipped_Stamp3 = 40,
  Complete_CloseoutDelay = 41,
  Complete_ClosedOut = 42,
};

// Legacy alias for backward compatibility (can be removed later)
enum struct GameTag : afterhours::TagId {
  // General tags
  IsOnShelf = static_cast<afterhours::TagId>(GeneralTag::IsOnShelf),
  IsGrabbed = static_cast<afterhours::TagId>(GeneralTag::IsGrabbed),
  IsBoxed = static_cast<afterhours::TagId>(GeneralTag::IsBoxed),
  IsBox = static_cast<afterhours::TagId>(GeneralTag::IsBox),
  IsInProgressOrder = static_cast<afterhours::TagId>(GeneralTag::IsInProgressOrder),
  IsOnConveyor = static_cast<afterhours::TagId>(GeneralTag::IsOnConveyor),
  IsSelectedOrder = static_cast<afterhours::TagId>(GeneralTag::IsSelectedOrder),

  // Order macro tags
  OrderMacro_Incoming = static_cast<afterhours::TagId>(OrderMacroTag::Incoming),
  OrderMacro_Opened = static_cast<afterhours::TagId>(OrderMacroTag::Opened),
  OrderMacro_RequestingItems = static_cast<afterhours::TagId>(OrderMacroTag::RequestingItems),
  OrderMacro_ReceivingItems = static_cast<afterhours::TagId>(OrderMacroTag::ReceivingItems),
  OrderMacro_ReadyToBox = static_cast<afterhours::TagId>(OrderMacroTag::ReadyToBox),
  OrderMacro_Boxing = static_cast<afterhours::TagId>(OrderMacroTag::Boxing),
  OrderMacro_Shipped = static_cast<afterhours::TagId>(OrderMacroTag::Shipped),
  OrderMacro_Complete = static_cast<afterhours::TagId>(OrderMacroTag::Complete),

  // Order micro tags
  OrderState_Incoming_Arrived = static_cast<afterhours::TagId>(OrderMicroTag::Incoming_Arrived),
  OrderState_Incoming_Backlogged = static_cast<afterhours::TagId>(OrderMicroTag::Incoming_Backlogged),
  OrderState_Opened_Active = static_cast<afterhours::TagId>(OrderMicroTag::Opened_Active),
  OrderState_Opened_Inactive = static_cast<afterhours::TagId>(OrderMicroTag::Opened_Inactive),
  OrderState_Requesting_NeedsInput = static_cast<afterhours::TagId>(OrderMicroTag::Requesting_NeedsInput),
  OrderState_Requesting_InputError = static_cast<afterhours::TagId>(OrderMicroTag::Requesting_InputError),
  OrderState_Requesting_AllRequested = static_cast<afterhours::TagId>(OrderMicroTag::Requesting_AllRequested),
  OrderState_Receiving_OnConveyorWaiting = static_cast<afterhours::TagId>(OrderMicroTag::Receiving_OnConveyorWaiting),
  OrderState_Receiving_OnConveyorMoving = static_cast<afterhours::TagId>(OrderMicroTag::Receiving_OnConveyorMoving),
  OrderState_Receiving_ReceivedToReady = static_cast<afterhours::TagId>(OrderMicroTag::Receiving_ReceivedToReady),
  OrderState_ReadyToBox_Staged = static_cast<afterhours::TagId>(OrderMicroTag::ReadyToBox_Staged),
  OrderState_ReadyToBox_Queued = static_cast<afterhours::TagId>(OrderMicroTag::ReadyToBox_Queued),
  OrderState_Boxing_FoldBox = static_cast<afterhours::TagId>(OrderMicroTag::Boxing_FoldBox),
  OrderState_Boxing_PutItems = static_cast<afterhours::TagId>(OrderMicroTag::Boxing_PutItems),
  OrderState_Boxing_Fold = static_cast<afterhours::TagId>(OrderMicroTag::Boxing_Fold),
  OrderState_Boxing_Tape = static_cast<afterhours::TagId>(OrderMicroTag::Boxing_Tape),
  OrderState_Boxing_Ship = static_cast<afterhours::TagId>(OrderMicroTag::Boxing_Ship),
  OrderState_Shipped_Stamp0 = static_cast<afterhours::TagId>(OrderMicroTag::Shipped_Stamp0),
  OrderState_Shipped_Stamp1 = static_cast<afterhours::TagId>(OrderMicroTag::Shipped_Stamp1),
  OrderState_Shipped_Stamp2 = static_cast<afterhours::TagId>(OrderMicroTag::Shipped_Stamp2),
  OrderState_Shipped_Stamp3 = static_cast<afterhours::TagId>(OrderMicroTag::Shipped_Stamp3),
  OrderState_Complete_CloseoutDelay = static_cast<afterhours::TagId>(OrderMicroTag::Complete_CloseoutDelay),
  OrderState_Complete_ClosedOut = static_cast<afterhours::TagId>(OrderMicroTag::Complete_ClosedOut),
};

// Order workflow component - authoritative source for order state
struct OrderWorkflow : afterhours::BaseComponent {
  OrderState state = OrderState::Incoming_Arrived;
  float time_in_state = 0.0f;
};

// Item count tracking components - explicit progress tracking
struct OrderRequiredCounts : afterhours::BaseComponent {
  std::map<ItemType, int> counts;
};

struct OrderRequestedCounts : afterhours::BaseComponent {
  std::map<ItemType, int> counts;
};

struct OrderReceivedCounts : afterhours::BaseComponent {
  std::map<ItemType, int> counts;
};

struct OrderBoxedCounts : afterhours::BaseComponent {
  std::map<ItemType, int> counts;
};

// Forward declare Order struct (defined in components.h)
struct Order;

// Include order state machine after components are defined
#include "order_state_machine.h"
