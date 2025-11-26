#pragma once

#include "../components.h"
#include "../input_wrapper.h"
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

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    if (!boxing_progress.order_id.has_value()) {
      for (int key = raylib::KEY_ONE; key <= raylib::KEY_NINE; ++key) {
        if (!game_input::IsKeyPressed(key)) {
          continue;
        }

        int order_index = key - raylib::KEY_ONE;
        if (order_index < 0 ||
            order_index >= static_cast<int>(queue.active_orders.size())) {
          break;
        }

        afterhours::EntityID order_id = queue.active_orders[order_index];
        if (order_id == -1) {
          break;
        }

        std::vector<ItemType> items_to_create;
        bool order_is_ready = false;

        for (Order &order : afterhours::EntityQuery()
                                .whereID(order_id)
                                .whereHasComponent<Order>()
                                .gen_as<Order>()) {
          if (all_items_ready(order) && !order.is_shipped) {
            order_is_ready = true;
            items_to_create = order.ready_items;
            order.is_complete = true;
            break;
          }
        }

        if (order_is_ready) {
          for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
            auto opt_entity = afterhours::EntityHelper::getEntityForID(item_id);
            if (opt_entity.has_value()) {
              opt_entity.value()->cleanup = true;
            }
          }
          boxing_progress.boxing_items.clear();

          for (ItemType item_type : items_to_create) {
            afterhours::Entity &boxing_item_entity =
                afterhours::EntityHelper::createEntity();
            BoxingItemStatus &boxing_item =
                boxing_item_entity.addComponent<BoxingItemStatus>();
            boxing_item.type = item_type;
            boxing_item.is_placed = false;
            boxing_progress.boxing_items.push_back(boxing_item_entity.id);
          }

          boxing_progress.order_id = order_id;
          boxing_progress.state = BoxingState::None;
          boxing_progress.items_placed = 0;
          return;
        }
        break;
      }
    }

    bool b_pressed = game_input::IsKeyPressed(raylib::KEY_B);

    if (b_pressed) {
      if (!boxing_progress.order_id.has_value()) {
        return;
      }

      if (boxing_progress.state == BoxingState::None) {
        boxing_progress.state = BoxingState::PutItems;
        return;
      }

      if (boxing_progress.state == BoxingState::FoldBox) {
        boxing_progress.state = BoxingState::PutItems;
        return;
      }
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
        // Find the first unplaced item and mark it as placed
        bool item_placed = false;
        for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
          if (item_placed)
            break;
          for (BoxingItemStatus &boxing_item :
               afterhours::EntityQuery()
                   .whereID(item_id)
                   .whereHasComponent<BoxingItemStatus>()
                   .gen_as<BoxingItemStatus>()) {
            if (!boxing_item.is_placed) {
              boxing_item.is_placed = true;
              boxing_progress.items_placed++;
              item_placed = true;
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
