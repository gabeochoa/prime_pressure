#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>
#include <map>

struct RenderWarehouseViewItems : WarehouseViewRenderSystem {
  void once(float) const override {
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
    float box_y = layout.warehouse.y / screen_height_f;
    float box_width = layout.warehouse.w / screen_width_f;
    float box_height = layout.warehouse.h / screen_height_f;

    float belt_y_pct = box_y + box_height * ui_constants::CONVEYOR_Y_PCT;
    float belt_y = ui_constants::pct_to_pixels_y(belt_y_pct, screen_height);
    float belt_height = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_BELT_HEIGHT_PCT, screen_height);
    float belt_start_x = ui_constants::pct_to_pixels_x(box_x, screen_width);
    float belt_end_x =
        ui_constants::pct_to_pixels_x(box_x + box_width, screen_width);
    float belt_width = belt_end_x - belt_start_x;

    std::map<afterhours::EntityID, std::map<float, std::map<ItemType, int>>>
        items_by_order_and_pos;
    std::map<afterhours::EntityID, int> order_index;
    std::map<afterhours::EntityID, std::map<ItemType, int>> conveyor_counts;

    // Build order index from slot components
    for (const afterhours::Entity &entity :
         afterhours::EntityQuery()
             .whereHasComponent<OrderSlot>()
             .whereHasComponent<Order>()
             .whereLambda([](const afterhours::Entity &e) {
               const OrderSlot &slot = e.get<OrderSlot>();
               return slot.index >= 0;
             })
             .gen()) {
      const OrderSlot &slot = entity.get<OrderSlot>();
      order_index[entity.id] = slot.index + 1; // 1-indexed for display
    }

    for (const ConveyorItem &conveyor_item :
         afterhours::EntityQuery()
             .whereHasComponent<ConveyorItem>()
             .whereHasTag(GameTag::IsOnConveyor)
             .gen_as<ConveyorItem>()) {
      items_by_order_and_pos[conveyor_item.order_id][conveyor_item.x_position]
                            [conveyor_item.type]++;
      conveyor_counts[conveyor_item.order_id][conveyor_item.type]++;
    }

    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);

    float item_start_y =
        belt_y - (belt_height / 2.0f) +
        ui_constants::pct_to_pixels_y(
            ui_constants::CONVEYOR_ITEM_VERTICAL_SPACING_PCT, screen_height);
    float item_spacing = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_ITEM_VERTICAL_SPACING_PCT, screen_height);

    // Request list only for the selected order
    if (selected_order.order_id.order_id.has_value()) {
      for (const Order &order :
           afterhours::EntityQuery()
               .whereID(selected_order.order_id.order_id.value())
               .whereHasComponent<Order>()
               .gen_as<Order>()) {
        std::map<ItemType, int> need = count_items(order.items);
        std::map<ItemType, int> selected_counts =
            count_items(order.selected_items);
        for (const auto &[it, cnt] : selected_counts) {
          need[it] -= cnt;
        }
        auto conv_it =
            conveyor_counts.find(selected_order.order_id.order_id.value());
        if (conv_it != conveyor_counts.end()) {
          for (const auto &[it, cnt] : conv_it->second) {
            need[it] -= cnt;
          }
        }

        std::string request_text;
        bool first_req = true;
        for (const auto &[it, cnt] : need) {
          if (cnt > 0) {
            if (!first_req) {
              request_text += ", ";
            }
            first_req = false;
            request_text += format_item_with_key(it);
            if (cnt > 1) {
              request_text += " x" + std::to_string(cnt);
            }
          }
        }

        if (first_req) {
          request_text = "All items accounted for";
        } else {
          request_text = "Request: " + request_text;
        }

        int req_font = ui_constants::pct_to_font_size(
            ui_constants::BODY_FONT_SIZE_PCT, screen_height);
        float req_x = ui_constants::pct_to_pixels_x(
            box_x + ui_constants::CONTENT_PADDING_PCT, screen_width);
        float req_y = belt_y - belt_height -
                      ui_constants::pct_to_pixels_y(
                          ui_constants::CONTENT_PADDING_PCT, screen_height);
        raylib::DrawTextEx(
            uiFont, request_text.c_str(), raylib::Vector2{req_x, req_y},
            static_cast<float>(req_font), 1.0f, ui_colors::TERMINAL_AMBER);
        break;
      }
    }

    int vertical_index = 0;
    for (const auto &[order_id, items_by_pos] : items_by_order_and_pos) {
      bool is_selected = selected_order.order_id.order_id.has_value() &&
                         selected_order.order_id.order_id.value() == order_id;

      for (const auto &[x_position, item_counts] : items_by_pos) {
        float screen_x = belt_start_x + (x_position * belt_width);
        float item_y = item_start_y + (vertical_index * item_spacing);

        bool has_moving_items = false;
        for (const ConveyorItem &conveyor_item :
             EQ().whereHasComponent<ConveyorItem>()
                 .whereHasTag(GameTag::IsOnConveyor)
                 .whereHasOrderID(order_id)
                 .gen_as<ConveyorItem>()) {
          if (std::abs(conveyor_item.x_position - x_position) < 0.001f &&
              conveyor_item.is_moving) {
            has_moving_items = true;
            break;
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

        raylib::Color item_color = raylib::Color{80, 90, 110, 255};
        if (!is_selected) {
          item_color = afterhours::colors::darken(item_color, 0.35f);
          item_color.a = static_cast<unsigned char>(
              static_cast<float>(item_color.a) * 0.55f); // lower opacity
        }
        if (has_moving_items && is_selected) {
          item_color = raylib::Color{49, 106, 197,
                                     255}; // Windows XP active accent color
        }

        raylib::DrawTextEx(
            uiFont, item_text.c_str(), raylib::Vector2{screen_x, item_y},
            static_cast<float>(body_font_size), 1.0f, item_color);
        auto badge_it = order_index.find(order_id);
        if (badge_it != order_index.end()) {
          std::string badge = "#" + std::to_string(badge_it->second);
          raylib::Vector2 text_size =
              raylib::MeasureTextEx(uiFont, item_text.c_str(),
                                    static_cast<float>(body_font_size), 1.0f);
          raylib::Color badge_bg = raylib::Color{10, 10, 10, 240};
          raylib::Color badge_fg = raylib::Color{255, 180, 0, 255};
          if (!is_selected) {
            float dim = 0.55f;
            badge_bg.a = static_cast<unsigned char>(
                static_cast<float>(badge_bg.a) * dim);
            badge_fg.a = static_cast<unsigned char>(
                static_cast<float>(badge_fg.a) * dim);
          }
          draw_badge(raylib::Vector2{screen_x + text_size.x + 8.0f,
                                     item_y - body_font_size * 0.2f},
                     badge, body_font_size, badge_bg, badge_fg);
        }
        vertical_index++;
      }
    }
  }
};
