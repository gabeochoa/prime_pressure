#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>
#include <optional>

struct RenderWarehouseViewOrderInfo : WarehouseViewRenderSystem {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;
    float screen_width_f = static_cast<float>(screen_width);
    float screen_height_f = static_cast<float>(screen_height);

    TriLayout layout = compute_tri_layout(screen_width, screen_height);

    float box_x = layout.warehouse.x / screen_width_f;
    float box_width = layout.warehouse.w / screen_width_f;
    float box_y = layout.warehouse.y / screen_height_f;

    draw_view_box(layout.warehouse, active_view.current_view,
                  ViewState::Warehouse);

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

    if (!selected_order.order_id.order_id.has_value()) {
      raylib::DrawTextEx(
          uiFont, "No order selected",
          raylib::Vector2{
              ui_constants::pct_to_pixels_x(
                  box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
              content_start_y_pixels},
          static_cast<float>(body_font_size), 1.0f, ui_colors::TERMINAL_GRAY);
      return;
    }

    // Find the slot index for the selected order
    int order_slot = -1;
    if (afterhours::EntityQuery()
            .whereID(selected_order.order_id.order_id.value())
            .whereHasComponent<OrderSlot>()
            .has_values()) {
      for (const OrderSlot &slot : afterhours::EntityQuery()
                                        .whereID(selected_order.order_id.order_id.value())
                                        .gen_as<OrderSlot>()) {
        order_slot = slot.index;
        break;
      }
    }

    if (order_slot >= 0) {
      std::string order_label = "Order #" + std::to_string(order_slot + 1);
      raylib::Vector2 label_pos{
          ui_constants::pct_to_pixels_x(
              box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
          content_start_y_pixels};
      raylib::DrawTextEx(
          uiFont, order_label.c_str(), label_pos,
          static_cast<float>(body_font_size), 1.0f,
          raylib::Color{60, 70, 90, 255});

      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(selected_order.order_id.order_id.value())
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        int total_items = static_cast<int>(order.items.size());
        int selected_items_count =
            static_cast<int>(order.selected_items.size());
        std::string progress_text = std::to_string(selected_items_count) +
                                    "/" + std::to_string(total_items) +
                                    " items";
        raylib::Vector2 badge_pos{
            ui_constants::pct_to_pixels_x(
                box_x + box_width - ui_constants::CONTENT_PADDING_PCT * 4.2f,
                screen_width),
            content_start_y_pixels - 4.0f};
        draw_badge(badge_pos, progress_text, instruction_font_size,
                   raylib::Color{255, 180, 0, 255},
                   raylib::Color{20, 20, 20, 255});
        break;
      }
    }
  }
};
