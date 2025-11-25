#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderWarehouseView : afterhours::System<> {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Warehouse) {
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

    raylib::DrawText("> WAREHOUSE SCREEN", static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + 10), 24, raylib::GREEN);

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    float y = box_y + 50.0f;

    if (selected_order.order_id.has_value()) {
      const afterhours::Entity &queue_entity =
          afterhours::EntityHelper::get_singleton<OrderQueue>();
      const OrderQueue &queue = queue_entity.get<OrderQueue>();

      int order_number = 1;
      for (afterhours::EntityID order_id : queue.active_orders) {
        if (order_id == selected_order.order_id.value()) {
          std::string order_label =
              "Selected Order: " + std::to_string(order_number);
          raylib::DrawText(order_label.c_str(), static_cast<int>(box_x + 20),
                           static_cast<int>(y), 20, raylib::WHITE);
          y += 40.0f;

          raylib::DrawText("Items needed:", static_cast<int>(box_x + 20),
                           static_cast<int>(y), 18, raylib::GRAY);
          y += 30.0f;

          for (const afterhours::Entity &order_entity :
               afterhours::EntityQuery()
                   .whereID(order_id)
                   .whereHasComponent<Order>()
                   .gen()) {
            const Order &order = order_entity.get<Order>();

            std::map<ItemType, int> item_counts;
            for (ItemType item_type : order.items) {
              item_counts[item_type]++;
            }

            std::map<ItemType, int> selected_counts;
            for (ItemType item_type : order.selected_items) {
              selected_counts[item_type]++;
            }

            for (const auto &[item_type, needed_count] : item_counts) {
              int selected_count = selected_counts[item_type];
              std::string item_name = item_type_to_string(item_type);
              std::string display_text = "  " + item_name;

              if (selected_count > 0) {
                display_text += " ✓";
                if (selected_count >= needed_count) {
                  raylib::DrawText(display_text.c_str(),
                                   static_cast<int>(box_x + 30),
                                   static_cast<int>(y), 16, raylib::GREEN);
                } else {
                  raylib::DrawText(display_text.c_str(),
                                   static_cast<int>(box_x + 30),
                                   static_cast<int>(y), 16, raylib::YELLOW);
                }
              } else {
                raylib::DrawText(display_text.c_str(),
                                 static_cast<int>(box_x + 30),
                                 static_cast<int>(y), 16, raylib::WHITE);
              }
              y += 25.0f;
            }
            break;
          }
          break;
        }
        order_number++;
      }
    } else {
      raylib::DrawText("Selected Order: None", static_cast<int>(box_x + 20),
                       static_cast<int>(y), 20, raylib::WHITE);
      y += 40.0f;

      raylib::DrawText("Items needed:", static_cast<int>(box_x + 20),
                       static_cast<int>(y), 18, raylib::GRAY);
      y += 30.0f;

      raylib::DrawText("(No order selected)", static_cast<int>(box_x + 30),
                       static_cast<int>(y), 16, raylib::DARKGRAY);
    }

    raylib::DrawText("[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
                     static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + box_height - 60), 16,
                     raylib::GRAY);
    raylib::DrawText(
        "[Type item names from order]", static_cast<int>(box_x + 10),
        static_cast<int>(box_y + box_height - 40), 16, raylib::GRAY);
  }
};
