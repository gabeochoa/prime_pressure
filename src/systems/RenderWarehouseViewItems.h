#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include <afterhours/ah.h>
#include <map>

struct RenderWarehouseViewItems : WarehouseViewRenderSystem {
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

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = ui_constants::BOX_X_PCT;
    float box_y = ui_constants::BOX_Y_PCT;
    float box_width = ui_constants::BOX_WIDTH_PCT;
    float box_height = ui_constants::BOX_HEIGHT_PCT;

    float belt_y_pct = box_y + box_height * ui_constants::CONVEYOR_Y_PCT;
    float belt_y = ui_constants::pct_to_pixels_y(belt_y_pct, screen_height);
    float belt_height = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_BELT_HEIGHT_PCT, screen_height);
    float belt_start_x = ui_constants::pct_to_pixels_x(box_x, screen_width);
    float belt_end_x =
        ui_constants::pct_to_pixels_x(box_x + box_width, screen_width);
    float belt_width = belt_end_x - belt_start_x;

    std::map<float, std::map<ItemType, int>> items_by_position;
    std::map<float, afterhours::EntityID> order_id_by_position;

    for (const ConveyorItem &conveyor_item :
         afterhours::EntityQuery()
             .whereHasComponent<ConveyorItem>()
             .whereHasTag(GameTag::IsOnConveyor)
             .gen_as<ConveyorItem>()) {
      bool order_has_been_selected = false;
      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(conveyor_item.order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        order_has_been_selected = order.has_been_selected;
        break;
      }
      if (!order_has_been_selected) {
        continue;
      }
      items_by_position[conveyor_item.x_position][conveyor_item.type]++;
      order_id_by_position[conveyor_item.x_position] = conveyor_item.order_id;
    }

    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);

    float item_start_y =
        belt_y - (belt_height / 2.0f) +
        ui_constants::pct_to_pixels_y(
            ui_constants::CONVEYOR_ITEM_VERTICAL_SPACING_PCT, screen_height);
    float item_spacing = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_ITEM_VERTICAL_SPACING_PCT, screen_height);

    int vertical_index = 0;
    for (const auto &[x_position, item_counts] : items_by_position) {
      float screen_x = belt_start_x + (x_position * belt_width);
      float item_y = item_start_y + (vertical_index * item_spacing);

      afterhours::EntityID order_id = order_id_by_position.at(x_position);
      bool is_selected = selected_order.order_id.has_value() &&
                         selected_order.order_id.value() == order_id;

      bool has_moving_items = false;
      for (const ConveyorItem &conveyor_item :
           EQ().whereHasComponent<ConveyorItem>()
               .whereHasTag(GameTag::IsOnConveyor)
               .whereHasOrderID(order_id)
               .gen_as<ConveyorItem>()) {
        if (std::abs(conveyor_item.x_position - x_position) < 0.001f) {
          if (conveyor_item.is_moving) {
            has_moving_items = true;
            break;
          }
        }
      }

      std::string item_text;
      bool first = true;
      for (const auto &[item_type, count] : item_counts) {
        if (!first) {
          item_text += ", ";
        }
        first = false;
        item_text += format_item_with_key(item_type);
        if (count > 1) {
          item_text += " x" + std::to_string(count);
        }
      }

      raylib::Color item_color = ui_colors::TERMINAL_GRAY;
      if (!is_selected) {
        item_color = afterhours::colors::darken(item_color, 0.5f);
      }
      if (has_moving_items && is_selected) {
        item_color = ui_colors::TERMINAL_GREEN;
      }

      raylib::DrawTextEx(uiFont, item_text.c_str(),
                         raylib::Vector2{screen_x, item_y},
                         static_cast<float>(body_font_size), 1.0f, item_color);
      vertical_index++;
    }
  }
};
