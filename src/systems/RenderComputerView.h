#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderComputerView : ComputerViewRenderSystem {
  void draw_order_list_item(const Order &order, int order_number, float x, float y,
                            int font_size, bool is_selected) const {
    std::map<ItemType, int> item_counts = count_items(order.items);
    std::map<ItemType, int> selected_counts = count_items(order.selected_items);

    std::string order_text = std::to_string(order_number) + ". ";
    
    // Check if order is fully ready
    bool is_ready = all_items_selected(order) && !order.is_shipped;
    
    raylib::Color text_color;
    if (order.is_shipped) {
      text_color = ui_colors::TERMINAL_GRAY;
    } else if (is_selected) {
      text_color = ui_colors::TERMINAL_GREEN; // Highlight selected
    } else if (is_ready) {
      text_color = ui_colors::TERMINAL_AMBER; // Ready to ship
    } else {
      text_color = ui_colors::TERMINAL_GRAY; // Normal
    }

    // Draw order number and status
    raylib::DrawTextEx(raylib::GetFontDefault(), order_text.c_str(), 
                       raylib::Vector2{x, y}, static_cast<float>(font_size), 1.0f, text_color);
                       
    // Draw items with status
    float current_x = x + raylib::MeasureTextEx(raylib::GetFontDefault(), order_text.c_str(), static_cast<float>(font_size), 1.0f).x;
    
    for (size_t i = 0; i < order.items.size(); ++i) {
      ItemType item_type = order.items[i];
      std::map<ItemType, int> current_item_counts;
      for (size_t j = 0; j <= i; ++j) {
        current_item_counts[order.items[j]]++;
      }

      bool is_completed = false;
      auto selected_it = selected_counts.find(item_type);
      auto current_it = current_item_counts.find(item_type);
      if (selected_it != selected_counts.end() &&
          current_it != current_item_counts.end()) {
        is_completed = selected_it->second >= current_it->second;
      }

      std::string item_display = format_item_with_key(item_type);
      raylib::Color item_color = is_completed ? ui_colors::TERMINAL_GREEN : text_color;
      
      if (i < order.items.size() - 1) {
        item_display += ", ";
      }
      
      raylib::DrawTextEx(raylib::GetFontDefault(), item_display.c_str(), 
                         raylib::Vector2{current_x, y}, static_cast<float>(font_size), 1.0f, item_color);
      current_x += raylib::MeasureTextEx(raylib::GetFontDefault(), item_display.c_str(), static_cast<float>(font_size), 1.0f).x;
    }

    if (is_ready) {
       raylib::DrawTextEx(raylib::GetFontDefault(), " [READY]", 
                         raylib::Vector2{current_x, y}, static_cast<float>(font_size), 1.0f, ui_colors::TERMINAL_AMBER);
    } else if (order.is_shipped) {
       raylib::DrawTextEx(raylib::GetFontDefault(), " [SHIPPED]", 
                         raylib::Vector2{current_x, y}, static_cast<float>(font_size), 1.0f, ui_colors::TERMINAL_GRAY);
    }
  }

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
                     active_view.current_view, ViewState::Computer,
                     screen_width, screen_height);

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    int header_font_size = ui_constants::pct_to_font_size(
        ui_constants::HEADER_FONT_SIZE_PCT, screen_height);
    float header_y_pixels =
        ui_constants::pct_to_pixels_y(header_y, screen_height);

    raylib::Vector2 header_text_size =
        raylib::MeasureTextEx(raylib::GetFontDefault(), "> [COMPUTER SCREEN]",
                              static_cast<float>(header_font_size), 1.0f);
    float header_bottom_pixels = header_y_pixels + header_text_size.y;

    float spacing_pixels = ui_constants::pct_to_pixels_y(
        ui_constants::HEADER_TO_CONTENT_SPACING_PCT, screen_height);
    float y_pixels = header_bottom_pixels + spacing_pixels;
    
    int order_number = 1;
    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    if (queue.in_progress_orders.empty()) {
      raylib::DrawTextEx(
          raylib::GetFontDefault(),
          "(No orders available)",
          raylib::Vector2{ui_constants::pct_to_pixels_x(
              box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
          ui_constants::pct_to_pixels_y(y_pixels / screen_height, screen_height)},
          static_cast<float>(instruction_font_size),
          1.0f,
          ui_colors::TERMINAL_GRAY);
    }

    for (afterhours::EntityID order_id : queue.in_progress_orders) {
      if (order_id == -1) {
        continue;
      }
      bool is_selected = selected_order.order_id.has_value() &&
                         selected_order.order_id.value() == order_id;

      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        
        draw_order_list_item(order, order_number, 
                             ui_constants::pct_to_pixels_x(box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
                             y_pixels,
                             body_font_size, is_selected);
                             
        y_pixels += ui_constants::pct_to_pixels_y(ui_constants::ORDER_ITEM_SPACING_PCT, screen_height);
        order_number++;
        break;
      }
    }

    float instruction_y =
        box_y + box_height - ui_constants::INSTRUCTION_PADDING_PCT;
    draw_instruction_text(
        "[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
        box_x + ui_constants::HEADER_PADDING_PCT, instruction_y, screen_width,
        screen_height);
    draw_instruction_text("[Press number key to select order]",
                          box_x + ui_constants::HEADER_PADDING_PCT,
                          instruction_y + ui_constants::HEADER_PADDING_PCT,
                          screen_width, screen_height);
  }
};
