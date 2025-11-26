#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderComputerView : ComputerViewRenderSystem {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    
    if (active_view.current_view != ViewState::Computer) {
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
    draw_view_header("COMPUTER SCREEN", header_x, header_y,
                     active_view.current_view, ViewState::Computer, screen_width,
                     screen_height);

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    float y = header_y + ui_constants::HEADER_FONT_SIZE_PCT +
              ui_constants::HEADER_TO_CONTENT_SPACING_PCT;
    int order_number = 1;
    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    if (queue.active_orders.empty()) {
      raylib::DrawText(
          "(No orders available)",
          static_cast<int>(
              ui_constants::pct_to_pixels_x(box_x + ui_constants::CONTENT_PADDING_PCT, screen_width)),
          static_cast<int>(
              ui_constants::pct_to_pixels_y(y, screen_height)),
          instruction_font_size,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
      y += ui_constants::ORDER_ITEM_SPACING_PCT;
    }

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

        bool is_ready = all_items_selected(order) && !order.is_shipped;
        if (is_ready) {
          order_text += " [READY]";
        } else if (order.is_shipped) {
          order_text += " [SHIPPED]";
        }

        raylib::Color text_color;
        if (order.is_shipped) {
          text_color = ui_constants::get_theme_color(
              afterhours::ui::Theme::Usage::Secondary);
        } else if (is_selected) {
          text_color = ui_constants::get_theme_color(
              afterhours::ui::Theme::Usage::Primary);
        } else if (is_ready) {
          text_color = ui_constants::get_theme_color(
              afterhours::ui::Theme::Usage::Accent);
        } else {
          text_color = ui_constants::get_theme_color(
              afterhours::ui::Theme::Usage::Font);
        }

        raylib::DrawText(
            order_text.c_str(),
            static_cast<int>(
                ui_constants::pct_to_pixels_x(box_x + ui_constants::CONTENT_PADDING_PCT, screen_width)),
            static_cast<int>(
                ui_constants::pct_to_pixels_y(y, screen_height)),
            body_font_size, text_color);
        y += ui_constants::ORDER_ITEM_SPACING_PCT;
        order_number++;
        break;
      }
    }

    float instruction_y = box_y + box_height - ui_constants::INSTRUCTION_PADDING_PCT;
    draw_instruction_text("[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
                           box_x + ui_constants::HEADER_PADDING_PCT,
                           instruction_y, screen_width, screen_height);
    draw_instruction_text("[Press number key to select order]",
                           box_x + ui_constants::HEADER_PADDING_PCT,
                           instruction_y + ui_constants::HEADER_PADDING_PCT,
                           screen_width, screen_height);
  }
};
