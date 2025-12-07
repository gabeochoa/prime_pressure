#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include <afterhours/ah.h>

struct ProcessBoxingInputSystem : afterhours::System<> {
  bool should_run(float) const override {
    return true; // Always run since all views are visible
  }

  void once(float) override {
    const ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

    if (!active_order.order_id.order_id.has_value()) {
      return;
    }

    BoxingProgress &boxing_progress = get_singleton_as<BoxingProgress>();

    if (!boxing_progress.order_id.has_value()) {
      start_boxing(boxing_progress, active_order.order_id.order_id.value());
      return;
    }

    handle_boxing_actions(boxing_progress);
  }

private:
  void cleanup_boxing_items(BoxingProgress &boxing_progress) const {
    for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
      auto opt_entity = afterhours::EntityHelper::getEntityForID(item_id);
      if (opt_entity.has_value()) {
        opt_entity.value()->cleanup = true;
      }
    }
    boxing_progress.boxing_items.clear();
  }

  void finish_shipping(BoxingProgress &boxing_progress, Order &order) const {
    order.set_ready_stamp_progress(0);
    cleanup_boxing_items(boxing_progress);
    boxing_progress.order_id.reset();
    boxing_progress.state = BoxingState::None;
    boxing_progress.items_placed = 0;
  }

  void create_boxing_items(BoxingProgress &boxing_progress,
                           const std::vector<ItemType> &items_to_create) const {
    for (ItemType item_type : items_to_create) {
      afterhours::Entity &boxing_item_entity =
          afterhours::EntityHelper::createEntity();
      BoxingItemStatus &boxing_item =
          boxing_item_entity.addComponent<BoxingItemStatus>();
      boxing_item.type = item_type;
      boxing_item.is_placed = false;
      boxing_progress.boxing_items.push_back(boxing_item_entity.id);
    }
  }

  void start_boxing(BoxingProgress &boxing_progress,
                    afterhours::EntityID order_id) const {
    Order *order_ptr = nullptr;
    for (afterhours::Entity &entity :
         afterhours::EntityQuery()
             .whereID(order_id)
             .whereHasComponent<Order>()
             .whereLambda([](const afterhours::Entity &entity) {
               const Order &o = entity.get<Order>();
               return all_items_ready(o) && !o.is_shipped();
             })
             .gen()) {
      order_ptr = &entity.get<Order>();
      break;
    }

    if (!order_ptr) {
      return;
    }

    std::vector<ItemType> items_to_create = order_ptr->ready_items;

    cleanup_boxing_items(boxing_progress);
    create_boxing_items(boxing_progress, items_to_create);

    boxing_progress.order_id = order_id;
    boxing_progress.state = BoxingState::PutItems;
    boxing_progress.items_placed = 0;
  }

  void handle_put_items(BoxingProgress &boxing_progress) const {
    bool item_placed = false;
    for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
      if (item_placed) {
        break;
      }
      BoxingItemStatus *boxing_item = nullptr;
      for (afterhours::Entity &entity :
           afterhours::EntityQuery()
               .whereID(item_id)
               .whereHasComponent<BoxingItemStatus>()
               .whereLambda([](const afterhours::Entity &entity) {
                 const BoxingItemStatus &bi = entity.get<BoxingItemStatus>();
                 return !bi.is_placed;
               })
               .gen()) {
        boxing_item = &entity.get<BoxingItemStatus>();
        break;
      }

      if (boxing_item != nullptr) {
        boxing_item->is_placed = true;
        boxing_progress.items_placed++;
        item_placed = true;
        int total_items = static_cast<int>(boxing_progress.boxing_items.size());
        if (boxing_progress.items_placed >= total_items) {
          boxing_progress.state = BoxingState::Fold;
        }
      }
    }
  }

  void handle_boxing_actions(BoxingProgress &boxing_progress) const {
    if (!boxing_progress.order_id.has_value()) {
      return;
    }

    for (Order &order : afterhours::EntityQuery()
                            .whereID(boxing_progress.order_id.value())
                            .whereHasComponent<Order>()
                            .gen_as<Order>()) {

      if (boxing_progress.state == BoxingState::PutItems &&
          game_input::IsKeyPressed(raylib::KEY_P)) {
        handle_put_items(boxing_progress);
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
        finish_shipping(boxing_progress, order);
        break;
      }
      break;
    }
  }
};
