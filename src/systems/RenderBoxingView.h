#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

static std::pair<std::string, raylib::Color>
get_boxing_status_text(const BoxingProgress &boxing_progress,
                       const Order &order) {
  switch (boxing_progress.state) {
  case BoxingState::FoldBox:
    return {
        "Press 'B' to fold the box",
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Secondary)};
  case BoxingState::PutItems: {
    int total_items = static_cast<int>(order.ready_items.size());
    std::string text = "Press 'P' to put items in box (" +
                       std::to_string(boxing_progress.items_placed) + "/" +
                       std::to_string(total_items) + ")";
    return {text, ui_constants::get_theme_color(
                      afterhours::ui::Theme::Usage::Secondary)};
  }
  case BoxingState::Fold:
    return {"Press 'F' to fold", ui_constants::get_theme_color(
                                     afterhours::ui::Theme::Usage::Secondary)};
  case BoxingState::Tape:
    return {"Press 'T' to tape", ui_constants::get_theme_color(
                                     afterhours::ui::Theme::Usage::Secondary)};
  case BoxingState::Ship:
    return {"Press 'S' to ship", ui_constants::get_theme_color(
                                     afterhours::ui::Theme::Usage::Primary)};
  case BoxingState::None:
    return {
        "Press 'B' to start boxing",
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Secondary)};
  }
  return {"",
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font)};
}

static void render_order_selection_list(float box_x, float &y,
                                        const OrderQueue &queue,
                                        const BoxingProgress &boxing_progress,
                                        int screen_width, int screen_height) {
  int body_font_size = ui_constants::pct_to_font_size(
      ui_constants::BODY_FONT_SIZE_PCT, screen_height);
  int instruction_font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  raylib::DrawTextEx(
      uiFont, "Select Order:",
      raylib::Vector2{
          ui_constants::pct_to_pixels_x(
              box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
          ui_constants::pct_to_pixels_y(y, screen_height)},
      static_cast<float>(body_font_size), 1.0f,
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  y += ui_constants::ORDER_ITEM_SPACING_PCT;

  int order_number = 1;
  for (afterhours::EntityID order_id : queue.in_progress_orders) {
    if (order_id == -1) {
      std::string order_text = std::to_string(order_number) + ". Order #" +
                               std::to_string(order_number);
      raylib::Color text_color =
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);

      raylib::DrawTextEx(
          uiFont, order_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f, text_color);

      std::string status_text = " - Slot not in progress";

      raylib::DrawTextEx(
          uiFont, status_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 3.0f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));

      y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
      order_number++;
      continue;
    }
    bool order_found = false;
    for (const Order &order : afterhours::EntityQuery()
                                  .whereID(order_id)
                                  .whereHasComponent<Order>()
                                  .gen_as<Order>()) {
      order_found = true;
      if (order.is_shipped) {
        std::string order_text = std::to_string(order_number) + ". Order #" +
                                 std::to_string(order_number);
        raylib::Color text_color =
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);

        raylib::DrawTextEx(
            uiFont, order_text.c_str(),
            raylib::Vector2{
                ui_constants::pct_to_pixels_x(
                    box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                    screen_width),
                ui_constants::pct_to_pixels_y(y, screen_height)},
            static_cast<float>(instruction_font_size), 1.0f, text_color);

        std::string status_text = " - Shipped";

        raylib::DrawTextEx(
            uiFont, status_text.c_str(),
            raylib::Vector2{
                ui_constants::pct_to_pixels_x(
                    box_x + ui_constants::CONTENT_PADDING_PCT * 3.0f,
                    screen_width),
                ui_constants::pct_to_pixels_y(y, screen_height)},
            static_cast<float>(instruction_font_size), 1.0f,
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));

        y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
        break;
      }

      bool is_ready = all_items_ready(order);
      bool is_selected = boxing_progress.order_id.has_value() &&
                         boxing_progress.order_id.value() == order_id;

      std::string order_text = std::to_string(order_number) + ". Order #" +
                               std::to_string(order_number);

      raylib::Color text_color =
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
      if (is_selected) {
        text_color = ui_constants::get_theme_color(
            afterhours::ui::Theme::Usage::Primary);
      } else if (is_ready) {
        text_color = ui_constants::get_theme_color(
            afterhours::ui::Theme::Usage::Primary);
      }

      raylib::DrawTextEx(
          uiFont, order_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f, text_color);

      std::string status_text;
      if (is_ready) {
        status_text = " - Ready";
      } else {
        int ready_count = static_cast<int>(order.ready_items.size());
        int total_count = static_cast<int>(order.items.size());
        status_text = " - Waiting for items (" + std::to_string(ready_count) +
                      "/" + std::to_string(total_count) + ")";
      }

      raylib::DrawTextEx(
          uiFont, status_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 3.0f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));

      y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
      break;
    }
    if (!order_found) {
      std::string order_text = std::to_string(order_number) + ". Order #" +
                               std::to_string(order_number);
      raylib::Color text_color =
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);

      raylib::DrawTextEx(
          uiFont, order_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f, text_color);

      std::string status_text = " - Slot not in progress";

      raylib::DrawTextEx(
          uiFont, status_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(
                              box_x + ui_constants::CONTENT_PADDING_PCT * 3.0f,
                              screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));

      y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
    }
    order_number++;
  }

  bool all_shipped_or_empty = true;
  for (afterhours::EntityID order_id : queue.in_progress_orders) {
    if (order_id == -1) {
      continue;
    }
    for (const Order &order : afterhours::EntityQuery()
                                  .whereID(order_id)
                                  .whereHasComponent<Order>()
                                  .gen_as<Order>()) {
      if (!order.is_shipped) {
        all_shipped_or_empty = false;
        break;
      }
      break;
    }
    if (!all_shipped_or_empty) {
      break;
    }
  }

  if (queue.in_progress_orders.empty() || all_shipped_or_empty) {
    raylib::DrawTextEx(
        uiFont, "No orders available",
        raylib::Vector2{
            ui_constants::pct_to_pixels_x(
                box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
            ui_constants::pct_to_pixels_y(y, screen_height)},
        static_cast<float>(instruction_font_size), 1.0f,
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  }
}

static void render_items_list(float left_x, float start_y,
                              const BoxingProgress &boxing_progress,
                              const OrderQueue &queue, int screen_width,
                              int screen_height) {
  int instruction_font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  float y = start_y;

  int items_to_display = 0;

  if (boxing_progress.order_id.has_value() &&
      !boxing_progress.boxing_items.empty()) {
    for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
      for (const BoxingItemStatus &boxing_item :
           afterhours::EntityQuery()
               .whereID(item_id)
               .whereHasComponent<BoxingItemStatus>()
               .gen_as<BoxingItemStatus>()) {
        if (!boxing_item.is_placed) {
          items_to_display++;
        }
        break;
      }
    }
  } else {
    for (afterhours::EntityID order_id : queue.in_progress_orders) {
      if (order_id == -1) {
        continue;
      }
      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        if (!order.is_shipped && !order.ready_items.empty()) {
          std::map<ItemType, int> item_counts = count_items(order.ready_items);
          items_to_display += static_cast<int>(item_counts.size());
        }
        break;
      }
    }
  }

  if (items_to_display == 0) {
    return;
  }

  raylib::DrawTextEx(
      uiFont, "Items:",
      raylib::Vector2{ui_constants::pct_to_pixels_x(left_x, screen_width),
                      ui_constants::pct_to_pixels_y(y, screen_height)},
      static_cast<float>(instruction_font_size), 1.0f,
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;

  if (boxing_progress.order_id.has_value() &&
      !boxing_progress.boxing_items.empty()) {
    std::map<ItemType, int> unplaced_counts;
    for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
      for (const BoxingItemStatus &boxing_item :
           afterhours::EntityQuery()
               .whereID(item_id)
               .whereHasComponent<BoxingItemStatus>()
               .gen_as<BoxingItemStatus>()) {
        if (!boxing_item.is_placed) {
          unplaced_counts[boxing_item.type]++;
        }
        break;
      }
    }

    for (const auto &[item_type, count] : unplaced_counts) {
      std::string item_text = "[ ] " + item_type_to_string(item_type);
      if (count > 1) {
        item_text += " x" + std::to_string(count);
      }
      raylib::DrawTextEx(
          uiFont, item_text.c_str(),
          raylib::Vector2{ui_constants::pct_to_pixels_x(left_x, screen_width),
                          ui_constants::pct_to_pixels_y(y, screen_height)},
          static_cast<float>(instruction_font_size), 1.0f,
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
      y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
    }
  } else {
    bool has_items = false;
    for (afterhours::EntityID order_id : queue.in_progress_orders) {
      if (order_id == -1) {
        continue;
      }
      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(order_id)
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        if (!order.is_shipped && !order.ready_items.empty()) {
          has_items = true;
          std::map<ItemType, int> item_counts = count_items(order.ready_items);
          for (const auto &[item_type, count] : item_counts) {
            std::string item_text =
                item_type_to_string(item_type) + " x" + std::to_string(count);
            raylib::DrawTextEx(
                uiFont, item_text.c_str(),
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(left_x, screen_width),
                    ui_constants::pct_to_pixels_y(y, screen_height)},
                static_cast<float>(instruction_font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
            y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
          }
        }
        break;
      }
    }
    if (!has_items) {
      return;
    }
  }
}

static void draw_dotted_rectangle(float x, float y, float width, float height,
                                  raylib::Color color, float dash_length = 5.0f,
                                  float gap_length = 5.0f) {
  float total_length = 0.0f;
  float current_x = x;
  float current_y = y;

  while (total_length < width) {
    float segment_length = std::min(dash_length, width - total_length);
    raylib::DrawLine(static_cast<int>(current_x), static_cast<int>(y),
                     static_cast<int>(current_x + segment_length),
                     static_cast<int>(y), color);
    current_x += segment_length + gap_length;
    total_length += segment_length + gap_length;
  }

  total_length = 0.0f;
  current_x = x;
  current_y = y + height;
  while (total_length < width) {
    float segment_length = std::min(dash_length, width - total_length);
    raylib::DrawLine(static_cast<int>(current_x), static_cast<int>(current_y),
                     static_cast<int>(current_x + segment_length),
                     static_cast<int>(current_y), color);
    current_x += segment_length + gap_length;
    total_length += segment_length + gap_length;
  }

  total_length = 0.0f;
  current_x = x;
  current_y = y;
  while (total_length < height) {
    float segment_length = std::min(dash_length, height - total_length);
    raylib::DrawLine(static_cast<int>(x), static_cast<int>(current_y),
                     static_cast<int>(x),
                     static_cast<int>(current_y + segment_length), color);
    current_y += segment_length + gap_length;
    total_length += segment_length + gap_length;
  }

  total_length = 0.0f;
  current_x = x + width;
  current_y = y;
  while (total_length < height) {
    float segment_length = std::min(dash_length, height - total_length);
    raylib::DrawLine(static_cast<int>(current_x), static_cast<int>(current_y),
                     static_cast<int>(current_x),
                     static_cast<int>(current_y + segment_length), color);
    current_y += segment_length + gap_length;
    total_length += segment_length + gap_length;
  }
}

static void render_box(float center_x_pct, float center_y_pct, int screen_width,
                       int screen_height,
                       const BoxingProgress &boxing_progress) {
  float center_x = ui_constants::pct_to_pixels_x(center_x_pct, screen_width);
  float center_y = ui_constants::pct_to_pixels_y(center_y_pct, screen_height);
  float box_width = ui_constants::pct_to_pixels_x(0.15f, screen_width);
  float box_height = ui_constants::pct_to_pixels_y(0.2f, screen_height);
  float box_x = center_x - box_width / 2.0f;
  float box_y = center_y - box_height / 2.0f;

  // Tan cardboard color (RGB: 210, 180, 140)
  raylib::Color box_color = raylib::Color{210, 180, 140, 255};

  if (boxing_progress.state == BoxingState::None) {
    draw_dotted_rectangle(box_x, box_y, box_width, box_height, box_color);
    return;
  }

  bool is_closed = boxing_progress.state >= BoxingState::Tape;

  if (is_closed) {
    raylib::DrawRectangle(static_cast<int>(box_x), static_cast<int>(box_y),
                          static_cast<int>(box_width),
                          static_cast<int>(box_height), box_color);

    // Draw off-white tape stripe only after pressing T (Ship state)
    if (boxing_progress.state == BoxingState::Ship) {
      raylib::Color tape_color = raylib::Color{250, 248, 240, 255}; // Off-white
      float stripe_width = box_width;
      float stripe_height = box_height * 0.08f; // 8% of box height
      float stripe_y = box_y + box_height / 2.0f - stripe_height / 2.0f;
      raylib::DrawRectangle(static_cast<int>(box_x), static_cast<int>(stripe_y),
                            static_cast<int>(stripe_width),
                            static_cast<int>(stripe_height), tape_color);
    }
  } else {
    raylib::DrawRectangleLines(static_cast<int>(box_x), static_cast<int>(box_y),
                               static_cast<int>(box_width),
                               static_cast<int>(box_height), box_color);
  }

  // Only show items in box when state is PutItems or Fold (after items are
  // placed)
  if (!is_closed && boxing_progress.state >= BoxingState::PutItems &&
      boxing_progress.order_id.has_value() &&
      !boxing_progress.boxing_items.empty()) {
    float item_y = box_y + 10.0f;
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
      for (const BoxingItemStatus &boxing_item :
           afterhours::EntityQuery()
               .whereID(item_id)
               .whereHasComponent<BoxingItemStatus>()
               .gen_as<BoxingItemStatus>()) {
        // Only show items that have been placed
        if (boxing_item.is_placed) {
          std::string item_text = item_type_to_string(boxing_item.type);
          raylib::DrawTextEx(
              uiFont, item_text.c_str(), raylib::Vector2{box_x + 10.0f, item_y},
              static_cast<float>(instruction_font_size) * 0.8f, 1.0f,
              ui_constants::get_theme_color(
                  afterhours::ui::Theme::Usage::Font));
          item_y += instruction_font_size * 0.8f;
        }
        break;
      }
    }
  }
}

static void render_boxing_progress(float box_x, float &y,
                                   const BoxingProgress &boxing_progress,
                                   const Order &order, int screen_width,
                                   int screen_height) {
  int body_font_size = ui_constants::pct_to_font_size(
      ui_constants::BODY_FONT_SIZE_PCT, screen_height);
  int instruction_font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  raylib::DrawTextEx(
      uiFont, "Boxing Order:",
      raylib::Vector2{
          ui_constants::pct_to_pixels_x(
              box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
          ui_constants::pct_to_pixels_y(y, screen_height)},
      static_cast<float>(body_font_size), 1.0f,
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  y += ui_constants::ORDER_ITEM_SPACING_PCT;

  auto [status_text, status_color] =
      get_boxing_status_text(boxing_progress, order);
  raylib::DrawTextEx(
      uiFont, status_text.c_str(),
      raylib::Vector2{
          ui_constants::pct_to_pixels_x(
              box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f, screen_width),
          ui_constants::pct_to_pixels_y(y, screen_height)},
      static_cast<float>(instruction_font_size), 1.0f, status_color);
  y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;
}

struct RenderBoxingView : BoxingViewRenderSystem {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Boxing) {
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
    draw_view_header("BOXING SCREEN", header_x, header_y,
                     active_view.current_view, ViewState::Boxing, screen_width,
                     screen_height, uiFont);

    const afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    const BoxingProgress &boxing_progress =
        boxing_progress_entity.get<BoxingProgress>();

    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();

    float content_start_y = header_y + ui_constants::HEADER_FONT_SIZE_PCT +
                            ui_constants::HEADER_TO_CONTENT_SPACING_PCT;

    float left_x = box_x + ui_constants::CONTENT_PADDING_PCT;
    render_items_list(left_x, content_start_y, boxing_progress, queue,
                      screen_width, screen_height);

    float center_x = box_x + box_width / 2.0f;
    float center_y = box_y + box_height / 2.0f;
    render_box(center_x, center_y, screen_width, screen_height,
               boxing_progress);

    float right_x = box_x + box_width * 0.6f;
    float y = content_start_y;

    if (!boxing_progress.order_id.has_value()) {
      render_order_selection_list(right_x, y, queue, boxing_progress,
                                  screen_width, screen_height);
    } else {
      for (const Order &order : afterhours::EntityQuery()
                                    .whereID(boxing_progress.order_id.value())
                                    .whereHasComponent<Order>()
                                    .gen_as<Order>()) {
        render_boxing_progress(right_x, y, boxing_progress, order, screen_width,
                               screen_height);
        break;
      }
    }

    float instruction_y =
        box_y + box_height - ui_constants::INSTRUCTION_PADDING_PCT;
    draw_instruction_text(
        "[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
        box_x + ui_constants::HEADER_PADDING_PCT, instruction_y, screen_width,
        screen_height, uiFont);
  }
};
