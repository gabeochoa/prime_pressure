#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderOrders : afterhours::System<> {
  void once(float) const override {
    raylib::DrawText("ORDERS:", 20, 50, 30, raylib::GREEN);

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    float y = 80.0f;
    for (afterhours::EntityID order_id : queue.active_orders) {
      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {

        std::string order_text = "Order: ";
        for (size_t i = 0; i < order.items.size(); ++i) {
          order_text += item_type_to_string(order.items[i]);
          if (i < order.items.size() - 1) {
            order_text += ", ";
          }
        }

        raylib::DrawText(order_text.c_str(), 20, static_cast<int>(y), 20,
                         raylib::WHITE);
        y += 30.0f;
        break;
      }
    }
  }
};
