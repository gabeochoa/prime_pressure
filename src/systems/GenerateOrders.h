#pragma once

#include "../components.h"
#include "../testing/test_input.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <magic_enum/magic_enum.hpp>
#include <random>

struct GenerateOrders : afterhours::System<> {
  static constexpr auto simple_items = magic_enum::enum_values<ItemType>();

  float time_since_last_order = 0.0f;
  float order_interval = ui_constants::ORDER_GENERATION_INTERVAL;
  bool initial_order_generated = false;

  void once(float dt) override {
    if (dt <= 0.0f) {
      return;
    }
    afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    OrderQueue &queue = queue_entity.get<OrderQueue>();

    if (test_input::test_mode && !initial_order_generated &&
        queue.active_orders.empty()) {
      generate_order(queue);
      initial_order_generated = true;
      return;
    }

    time_since_last_order += dt;

    if (static_cast<int>(queue.active_orders.size()) >=
        queue.max_active_orders) {
      return;
    }

    if (time_since_last_order < order_interval) {
      return;
    }

    generate_order(queue);
    time_since_last_order = 0.0f;
  }

private:
  void generate_order(OrderQueue &queue) {
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

    queue.active_orders.push_back(order_entity.id);
  }
};
