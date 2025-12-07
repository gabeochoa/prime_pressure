#pragma once

#include "../components.h"
#include "../testing/test_input.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <magic_enum/magic_enum.hpp>
#include <random>

struct GenerateOrdersSystem : afterhours::System<> {
  static constexpr auto simple_items = magic_enum::enum_values<ItemType>();
  static constexpr int MAX_IN_PROGRESS_ORDERS = 3;

  float time_since_last_order = 0.0f;
  float order_interval = ui_constants::ORDER_GENERATION_INTERVAL;
  bool initial_order_generated = false;

  void once(float dt) override {
    if (dt <= 0.0f) {
      return;
    }

    if (test_input::test_mode && !initial_order_generated &&
        !has_any_in_progress_orders()) {
      generate_order();
      initial_order_generated = true;
      return;
    }

    time_since_last_order += dt;

    if (count_in_progress_orders() >= MAX_IN_PROGRESS_ORDERS) {
      return;
    }

    if (time_since_last_order < order_interval) {
      return;
    }

    int free_slot = find_free_slot();
    if (free_slot >= 0) {
      generate_order_to_slot(free_slot);
    }
    time_since_last_order = 0.0f;
  }

private:
  bool has_any_in_progress_orders() const {
    return afterhours::EntityQuery()
        .whereHasComponent<OrderSlot>()
        .whereLambda([](const afterhours::Entity &entity) {
          const OrderSlot &slot = entity.get<OrderSlot>();
          return slot.index >= 0;
        })
        .has_values();
  }

  size_t count_in_progress_orders() const {
    return afterhours::EntityQuery()
        .whereHasComponent<OrderSlot>()
        .whereLambda([](const afterhours::Entity &entity) {
          const OrderSlot &slot = entity.get<OrderSlot>();
          return slot.index >= 0;
        })
        .gen_count();
  }

  int find_free_slot() const {
    for (int i = 0; i < MAX_IN_PROGRESS_ORDERS; ++i) {
      bool slot_used = afterhours::EntityQuery()
                           .whereHasComponent<OrderSlot>()
                           .whereLambda([i](const afterhours::Entity &entity) {
                             const OrderSlot &slot = entity.get<OrderSlot>();
                             return slot.index == i;
                           })
                           .has_values();

      if (!slot_used) {
        return i;
      }
    }
    return -1;
  }

  void generate_order_to_slot(int slot_index) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> item_count_dist(1, 3);
    std::uniform_int_distribution<> item_index_dist(
        0, static_cast<int>(simple_items.size() - 1));

    int item_count = item_count_dist(gen);
    std::vector<ItemType> order_items;

    for (int i = 0; i < item_count; ++i) {
      ItemType selected_item = simple_items[item_index_dist(gen)];
      order_items.push_back(selected_item);
    }

    afterhours::Entity &order_entity = afterhours::EntityHelper::createEntity();
    Order &order = order_entity.addComponent<Order>();
    order.items = order_items;

    OrderSlot &slot = order_entity.addComponent<OrderSlot>();
    slot.index = slot_index;
  }

  void generate_order() {
    int free_slot = find_free_slot();
    if (free_slot < 0) {
      return;
    }
    generate_order_to_slot(free_slot);
  }
};
