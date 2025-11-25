#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderComputerView : afterhours::System<> {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Computer) {
      return;
    }

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = 20.0f;
    float box_y = 60.0f;
    float box_width = static_cast<float>(screen_width) - 40.0f;
    float box_height = static_cast<float>(screen_height) - 200.0f;

    raylib::DrawRectangleLines(static_cast<int>(box_x), static_cast<int>(box_y),
                               static_cast<int>(box_width),
                               static_cast<int>(box_height), raylib::GREEN);

    raylib::DrawText("> COMPUTER SCREEN", static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + 10), 24, raylib::GREEN);

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    float y = box_y + 50.0f;
    int order_number = 1;

    for (afterhours::EntityID order_id : queue.active_orders) {
      bool is_selected = selected_order.order_id.has_value() &&
                         selected_order.order_id.value() == order_id;

      for (const afterhours::Entity &order_entity :
           afterhours::EntityQuery()
               .whereID(order_id)
               .whereHasComponent<Order>()
               .gen()) {
        const Order &order = order_entity.get<Order>();

        std::string order_text = std::to_string(order_number) + ". ";
        for (size_t i = 0; i < order.items.size(); ++i) {
          order_text += item_type_to_string(order.items[i]);
          if (i < order.items.size() - 1) {
            order_text += ", ";
          }
        }

        raylib::Color text_color = is_selected ? raylib::GREEN : raylib::WHITE;
        raylib::DrawText(order_text.c_str(), static_cast<int>(box_x + 20),
                         static_cast<int>(y), 20, text_color);
        y += 30.0f;
        order_number++;
        break;
      }
    }

    raylib::DrawText("[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
                     static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + box_height - 60), 16,
                     raylib::GRAY);
    raylib::DrawText(
        "[Press number key to select order]", static_cast<int>(box_x + 10),
        static_cast<int>(box_y + box_height - 40), 16, raylib::GRAY);
  }
};
