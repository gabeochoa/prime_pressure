#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderOrders : afterhours::System<> {
  void once(float) const override {
    raylib::DrawText("ORDERS:", 20, 50, 30, raylib::GREEN);

    float y = 80.0f;
    for (const afterhours::Entity &entity :
         afterhours::EntityQuery()
             .whereHasComponent<OrderSlot>()
             .whereHasComponent<Order>()
             .whereLambda([](const afterhours::Entity &e) {
               const OrderSlot &slot = e.get<OrderSlot>();
               return slot.index >= 0;
             })
             .gen()) {
      const Order &order = entity.get<Order>();
      const OrderSlot &slot = entity.get<OrderSlot>();
      std::string order_text =
          "Order #" + std::to_string(slot.index + 1) + ": ";
      for (size_t i = 0; i < order.items.size(); ++i) {
        order_text += item_type_to_string(order.items[i]);
        if (i < order.items.size() - 1) {
          order_text += ", ";
        }
      }

      raylib::DrawText(order_text.c_str(), 20, static_cast<int>(y), 20,
                       raylib::WHITE);
      y += 30.0f;
    }
  }
};
