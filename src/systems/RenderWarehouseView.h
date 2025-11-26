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

    float belt_y_pct = box_y + box_height * ui_constants::CONVEYOR_Y_PCT;
    float belt_y = ui_constants::pct_to_pixels_y(belt_y_pct, screen_height);
    float belt_height = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_BELT_HEIGHT_PCT, screen_height);
    float belt_start_x = ui_constants::pct_to_pixels_x(box_x, screen_width);
    float belt_end_x =
        ui_constants::pct_to_pixels_x(box_x + box_width, screen_width);
    float belt_width = belt_end_x - belt_start_x;

    raylib::Color belt_color =
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Background);
    belt_color = raylib::Color{belt_color.r, belt_color.g, belt_color.b, 200};
    raylib::DrawRectangle(static_cast<int>(belt_start_x),
                          static_cast<int>(belt_y - belt_height / 2.0f),
                          static_cast<int>(belt_width),
                          static_cast<int>(belt_height), belt_color);

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
          std::string order_label = "Order #" + std::to_string(order_number);
          raylib::DrawTextEx(
              uiFont, order_label.c_str(),
              raylib::Vector2{
                  ui_constants::pct_to_pixels_x(
                      box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
                  content_start_y_pixels},
              static_cast<float>(body_font_size), 1.0f,
              ui_constants::get_theme_color(
                  afterhours::ui::Theme::Usage::Font));

          for (const Order &order : afterhours::EntityQuery()
                                        .whereID(order_id)
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
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(
                        box_x + box_width -
                            ui_constants::CONTENT_PADDING_PCT * 5.0f,
                        screen_width),
                    content_start_y_pixels},
                static_cast<float>(instruction_font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
            break;
          }
          break;
        }
        order_number++;
      }
    }

    if (selected_order.order_id.has_value()) {
      float start_y =
          belt_y - (belt_height / 2.0f) +
          ui_constants::pct_to_pixels_y(
              ui_constants::CONVEYOR_ITEM_VERTICAL_SPACING_PCT, screen_height);

      for (const ConveyorItem &conveyor_item :
           afterhours::EntityQuery()
               .whereHasComponent<ConveyorItem>()
               .gen_as<ConveyorItem>()) {

        if (conveyor_item.order_id == selected_order.order_id.value()) {
          float screen_x =
              belt_start_x + (conveyor_item.x_position * belt_width);
          float item_y =
              start_y + (conveyor_item.vertical_index *
                         ui_constants::pct_to_pixels_y(
                             ui_constants::CONVEYOR_ITEM_VERTICAL_SPACING_PCT,
                             screen_height));
          std::string item_text = item_type_to_string(conveyor_item.type);

          raylib::Color item_color =
              ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
          if (conveyor_item.is_moving) {
            item_color = ui_constants::get_theme_color(
                afterhours::ui::Theme::Usage::Primary);
          }

          raylib::DrawTextEx(
              uiFont, item_text.c_str(), raylib::Vector2{screen_x, item_y},
              static_cast<float>(body_font_size), 1.0f, item_color);
        }
      }
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
