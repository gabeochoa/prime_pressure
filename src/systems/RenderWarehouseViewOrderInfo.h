#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include <afterhours/ah.h>
#include <optional>

struct RenderWarehouseViewOrderInfo : WarehouseViewRenderSystem {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Warehouse) {
      return;
    }

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    if (!selected_order.order_id.has_value()) {
      return;
    }

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = ui_constants::BOX_X_PCT;
    float box_width = ui_constants::BOX_WIDTH_PCT;
    float box_y = ui_constants::BOX_Y_PCT;
    float box_height = ui_constants::BOX_HEIGHT_PCT;

    draw_view_box(box_x, box_y, box_width, box_height, screen_width, screen_height);

    float header_x = box_x + ui_constants::HEADER_PADDING_PCT;
    float header_y = box_y + ui_constants::HEADER_PADDING_PCT;
    
    draw_view_header("WAREHOUSE SCREEN", header_x, header_y,
                     active_view.current_view, ViewState::Warehouse,
                     screen_width, screen_height, uiFont);

    int header_font_size = ui_constants::pct_to_font_size(
        ui_constants::HEADER_FONT_SIZE_PCT, screen_height);
    float header_y_pixels =
        ui_constants::pct_to_pixels_y(header_y, screen_height);

    raylib::Vector2 header_text_size =
        raylib::MeasureTextEx(uiFont, "> [WAREHOUSE SCREEN]",
                              static_cast<float>(header_font_size), 1.0f);
    float header_bottom_pixels = header_y_pixels + header_text_size.y;

    float spacing_pixels = ui_constants::pct_to_pixels_y(
        ui_constants::HEADER_TO_CONTENT_SPACING_PCT, screen_height);
    float content_start_y_pixels = header_bottom_pixels + spacing_pixels;

    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    int order_number = 1;
    for (afterhours::EntityID order_id : queue.in_progress_orders) {
      if (order_id == -1) {
        order_number++;
        continue;
      }
      if (order_id == selected_order.order_id.value()) {
        std::string order_label = "Order #" + std::to_string(order_number);
        raylib::DrawTextEx(
            uiFont, order_label.c_str(),
            raylib::Vector2{
                ui_constants::pct_to_pixels_x(
                    box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
                content_start_y_pixels},
            static_cast<float>(body_font_size), 1.0f,
            ui_colors::TERMINAL_GREEN);

        for (const Order &order : afterhours::EntityQuery()
                                      .whereID(selected_order.order_id.value())
                                      .whereHasComponent<Order>()
                                      .gen_as<Order>()) {
          int total_items = static_cast<int>(order.items.size());
          int selected_items_count =
              static_cast<int>(order.selected_items.size());
          std::string progress_text = std::to_string(selected_items_count) +
                                      "/" + std::to_string(total_items) +
                                      " items";
          raylib::DrawTextEx(
              uiFont, progress_text.c_str(),
              raylib::Vector2{ui_constants::pct_to_pixels_x(
                                  box_x + box_width -
                                      ui_constants::CONTENT_PADDING_PCT * 5.0f,
                                  screen_width),
                              content_start_y_pixels},
              static_cast<float>(instruction_font_size), 1.0f,
              ui_colors::TERMINAL_AMBER);
          break;
        }
        break;
      }
      order_number++;
    }
  }
};
