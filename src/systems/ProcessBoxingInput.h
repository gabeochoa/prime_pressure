#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include "../log/log.h"
#include <afterhours/ah.h>

struct ProcessBoxingInput : afterhours::System<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Boxing;
  }

  void once(float) override {
    afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    BoxingProgress &boxing_progress =
        boxing_progress_entity.get<BoxingProgress>();

    bool b_pressed = game_input::IsKeyPressed(raylib::KEY_B);
    if (b_pressed) {
      log_info("ProcessBoxingInput: B PRESSED! order_id={}, state={}",
               boxing_progress.order_id.has_value()
                   ? boxing_progress.order_id.value()
                   : -1,
               static_cast<int>(boxing_progress.state));
    }

    if (b_pressed) {
      if (!boxing_progress.order_id.has_value()) {
        log_info(
            "ProcessBoxingInput: No order selected, looking for ready order");
        const afterhours::Entity &queue_entity =
            afterhours::EntityHelper::get_singleton<OrderQueue>();
        const OrderQueue &queue = queue_entity.get<OrderQueue>();

        afterhours::EntityID ready_order_id = -1;
        for (afterhours::EntityID order_id : queue.active_orders) {
          for (const Order &order : afterhours::EntityQuery()
                                        .whereID(order_id)
                                        .whereHasComponent<Order>()
                                        .gen_as<Order>()) {
            bool all_selected = all_items_selected(order);
            log_info("ProcessBoxingInput: Checking order {}, all_selected={}, "
                     "is_shipped={}",
                     order_id, all_selected, order.is_shipped);
            if (all_selected && !order.is_shipped) {
              ready_order_id = order_id;
              break;
            }
            break;
          }
          if (ready_order_id != -1) {
            break;
          }
        }

        if (ready_order_id != -1) {
          log_info("ProcessBoxingInput: Found ready order {}, selecting and "
                   "setting state to FoldBox",
                   ready_order_id);
          for (Order &order : afterhours::EntityQuery()
                                  .whereID(ready_order_id)
                                  .whereHasComponent<Order>()
                                  .gen_as<Order>()) {
            for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
              auto opt_entity =
                  afterhours::EntityHelper::getEntityForID(item_id);
              if (opt_entity.has_value()) {
                opt_entity.value()->cleanup = true;
              }
            }
            boxing_progress.boxing_items.clear();

            for (ItemType item_type : order.selected_items) {
              afterhours::Entity &boxing_item_entity =
                  afterhours::EntityHelper::createEntity();
              BoxingItemStatus &boxing_item =
                  boxing_item_entity.addComponent<BoxingItemStatus>();
              boxing_item.type = item_type;
              boxing_item.is_placed = false;
              boxing_progress.boxing_items.push_back(boxing_item_entity.id);
            }

            order.is_complete = true;
            boxing_progress.order_id = ready_order_id;
            boxing_progress.state = BoxingState::FoldBox;
            boxing_progress.items_placed = 0;
            log_info(
                "ProcessBoxingInput: Order selected, state set to FoldBox");
            return;
          }
        }
        log_info("ProcessBoxingInput: No ready order found");
        return;
      }

      if (boxing_progress.state == BoxingState::None) {
        log_info("ProcessBoxingInput: State is None, transitioning to FoldBox");
        boxing_progress.state = BoxingState::FoldBox;
        return;
      }

      if (boxing_progress.state == BoxingState::FoldBox) {
        log_info(
            "ProcessBoxingInput: State is FoldBox, transitioning to PutItems");
        boxing_progress.state = BoxingState::PutItems;
        return;
      }

      log_info("ProcessBoxingInput: B pressed but state is {}, no action taken",
               static_cast<int>(boxing_progress.state));
    }

    if (!boxing_progress.order_id.has_value()) {
      return;
    }

    for (Order &order : afterhours::EntityQuery()
                            .whereID(boxing_progress.order_id.value())
                            .whereHasComponent<Order>()
                            .gen_as<Order>()) {

      if (boxing_progress.state == BoxingState::PutItems &&
          game_input::IsKeyPressed(raylib::KEY_P)) {
        for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
          for (BoxingItemStatus &boxing_item :
               afterhours::EntityQuery()
                   .whereID(item_id)
                   .whereHasComponent<BoxingItemStatus>()
                   .gen_as<BoxingItemStatus>()) {
            if (!boxing_item.is_placed) {
              boxing_item.is_placed = true;
              boxing_progress.items_placed++;
              int total_items =
                  static_cast<int>(boxing_progress.boxing_items.size());
              if (boxing_progress.items_placed >= total_items) {
                boxing_progress.state = BoxingState::Fold;
              }
              break;
            }
          }
        }
        break;
      }

      if (boxing_progress.state == BoxingState::Fold &&
          game_input::IsKeyPressed(raylib::KEY_F)) {
        boxing_progress.state = BoxingState::Tape;
        break;
      }

      if (boxing_progress.state == BoxingState::Tape &&
          game_input::IsKeyPressed(raylib::KEY_T)) {
        boxing_progress.state = BoxingState::Ship;
        break;
      }

      if (boxing_progress.state == BoxingState::Ship &&
          game_input::IsKeyPressed(raylib::KEY_S)) {
        order.is_shipped = true;
        for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
          auto opt_entity = afterhours::EntityHelper::getEntityForID(item_id);
          if (opt_entity.has_value()) {
            opt_entity.value()->cleanup = true;
          }
        }
        boxing_progress.boxing_items.clear();
        boxing_progress.order_id.reset();
        boxing_progress.state = BoxingState::None;
        boxing_progress.items_placed = 0;
        break;
      }
      break;
    }
  }
};
