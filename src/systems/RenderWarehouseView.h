#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderWarehouseView : WarehouseViewRenderSystem {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Warehouse) {
      return;
    }

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = ui_constants::BOX_X_PCT;
    float box_y = ui_constants::BOX_Y_PCT;
    float box_width = ui_constants::BOX_WIDTH_PCT;
    float box_height = ui_constants::BOX_HEIGHT_PCT;

    draw_view_box(box_x, box_y, box_width, box_height, screen_width,
                  screen_height);

    float header_x = box_x + ui_constants::HEADER_PADDING_PCT;
    float header_y = box_y + ui_constants::HEADER_PADDING_PCT;
    draw_view_header("WAREHOUSE SCREEN", header_x, header_y,
                     active_view.current_view, ViewState::Warehouse,
                     screen_width, screen_height, uiFont);

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    float y = header_y + ui_constants::HEADER_FONT_SIZE_PCT +
              ui_constants::HEADER_TO_CONTENT_SPACING_PCT;
    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    if (selected_order.order_id.has_value()) {
      const afterhours::Entity &queue_entity =
          afterhours::EntityHelper::get_singleton<OrderQueue>();
      const OrderQueue &queue = queue_entity.get<OrderQueue>();

      int order_number = 1;
      for (afterhours::EntityID order_id : queue.active_orders) {
        if (order_id == selected_order.order_id.value()) {
          std::string order_label =
              "Selected Order: " + std::to_string(order_number);
          raylib::DrawTextEx(
              uiFont, order_label.c_str(),
              raylib::Vector2{
                  ui_constants::pct_to_pixels_x(
                      box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
                  ui_constants::pct_to_pixels_y(y, screen_height)},
              static_cast<float>(body_font_size), 1.0f,
              ui_constants::get_theme_color(
                  afterhours::ui::Theme::Usage::Font));
          y += ui_constants::ORDER_ITEM_SPACING_PCT;

          for (const afterhours::Entity &order_entity :
               afterhours::EntityQuery()
                   .whereID(order_id)
                   .whereHasComponent<Order>()
                   .gen()) {
            const Order &order = order_entity.get<Order>();

            int total_items = static_cast<int>(order.items.size());
            int selected_items_count =
                static_cast<int>(order.selected_items.size());
            std::string progress_text =
                "Items collected: " + std::to_string(selected_items_count) +
                "/" + std::to_string(total_items);
            raylib::DrawTextEx(
                uiFont, progress_text.c_str(),
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(
                        box_x + ui_constants::CONTENT_PADDING_PCT,
                        screen_width),
                    ui_constants::pct_to_pixels_y(y, screen_height)},
                static_cast<float>(instruction_font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
            y += ui_constants::ORDER_ITEM_SPACING_PCT;

            raylib::DrawTextEx(
                uiFont, "Items needed:",
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(
                        box_x + ui_constants::CONTENT_PADDING_PCT,
                        screen_width),
                    ui_constants::pct_to_pixels_y(y, screen_height)},
                static_cast<float>(instruction_font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
            y += ui_constants::ORDER_ITEM_SPACING_PCT;

            std::map<ItemType, int> item_counts = count_items(order.items);
            std::map<ItemType, int> selected_counts =
                count_items(order.selected_items);

            for (const auto &[item_type, needed_count] : item_counts) {
              int selected_count = selected_counts[item_type];
              std::string item_name = item_type_to_string(item_type);
              std::string display_text = "  " + item_name;

              raylib::Color item_color = ui_constants::get_theme_color(
                  afterhours::ui::Theme::Usage::Font);
              if (selected_count > 0) {
                display_text += " ✓";
                item_color = (selected_count >= needed_count)
                                 ? ui_constants::get_theme_color(
                                       afterhours::ui::Theme::Usage::Primary)
                                 : ui_constants::get_theme_color(
                                       afterhours::ui::Theme::Usage::Secondary);
              }

              raylib::DrawTextEx(
                  uiFont, display_text.c_str(),
                  raylib::Vector2{
                      ui_constants::pct_to_pixels_x(
                          box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                          screen_width),
                      ui_constants::pct_to_pixels_y(y, screen_height)},
                  static_cast<float>(instruction_font_size), 1.0f, item_color);
              y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;
            }
            break;
          }
          break;
        }
        order_number++;
      }
    } else {
      raylib::DrawTextEx(
          uiFont, "Selected Order: None",
          raylib::Vector2{
              ui_constants::pct_to_pixels_x(
                  box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
              ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(body_font_size), 1.0f,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
      y += ui_constants::ORDER_ITEM_SPACING_PCT;

      raylib::DrawTextEx(
          uiFont, "Items needed:",
          raylib::Vector2{
              ui_constants::pct_to_pixels_x(
                  box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
              ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
      y += ui_constants::ORDER_ITEM_SPACING_PCT;

      raylib::Color dimmed_color =
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
      dimmed_color = raylib::Color{
          static_cast<unsigned char>(dimmed_color.r / 2),
          static_cast<unsigned char>(dimmed_color.g / 2),
          static_cast<unsigned char>(dimmed_color.b / 2), dimmed_color.a};
      raylib::DrawTextEx(
          uiFont, "(No order selected)",
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f, dimmed_color);
    }

    float instruction_y =
        box_y + box_height - ui_constants::INSTRUCTION_PADDING_PCT;
    draw_instruction_text(
        "[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
        box_x + ui_constants::HEADER_PADDING_PCT, instruction_y, screen_width,
        screen_height, uiFont);
    draw_instruction_text("[Type item names from order]",
                          box_x + ui_constants::HEADER_PADDING_PCT,
                          instruction_y + ui_constants::HEADER_PADDING_PCT,
                          screen_width, screen_height, uiFont);
  }
};
