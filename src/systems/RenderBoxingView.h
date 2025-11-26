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
    int total_items = static_cast<int>(order.selected_items.size());
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
    return {"",
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font)};
  }
  return {"",
          ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font)};
}

static void render_order_selection_list(float box_x, float &y,
                                        const OrderQueue &queue,
                                        int screen_width, int screen_height) {
  int body_font_size = ui_constants::pct_to_font_size(
      ui_constants::BODY_FONT_SIZE_PCT, screen_height);
  int instruction_font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  raylib::DrawText(
      "Select order to box:",
      static_cast<int>(ui_constants::pct_to_pixels_x(
          box_x + ui_constants::CONTENT_PADDING_PCT, screen_width)),
      static_cast<int>(ui_constants::pct_to_pixels_y(y, screen_height)),
      body_font_size,
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  y += ui_constants::ORDER_ITEM_SPACING_PCT;

  int order_number = 1;
  for (afterhours::EntityID order_id : queue.active_orders) {
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(order_id)
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();

      if (all_items_selected(order) && !order.is_shipped) {
        std::string order_text =
            std::to_string(order_number) + ". Ready to box";
        raylib::DrawText(
            order_text.c_str(),
            static_cast<int>(ui_constants::pct_to_pixels_x(
                box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f,
                screen_width)),
            static_cast<int>(ui_constants::pct_to_pixels_y(y, screen_height)),
            instruction_font_size,
            ui_constants::get_theme_color(
                afterhours::ui::Theme::Usage::Primary));
        y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;
      }
      break;
    }
    order_number++;
  }

  draw_instruction_text("[Press number key to select order]",
                        box_x + ui_constants::CONTENT_PADDING_PCT,
                        y + ui_constants::HEADER_PADDING_PCT, screen_width,
                        screen_height);
}

static void render_boxing_progress(float box_x, float &y,
                                   const BoxingProgress &boxing_progress,
                                   const Order &order, int screen_width,
                                   int screen_height) {
  int body_font_size = ui_constants::pct_to_font_size(
      ui_constants::BODY_FONT_SIZE_PCT, screen_height);
  int instruction_font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  raylib::DrawText(
      "Boxing Order:",
      static_cast<int>(ui_constants::pct_to_pixels_x(
          box_x + ui_constants::CONTENT_PADDING_PCT, screen_width)),
      static_cast<int>(ui_constants::pct_to_pixels_y(y, screen_height)),
      body_font_size,
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  y += ui_constants::ORDER_ITEM_SPACING_PCT;

  auto [status_text, status_color] =
      get_boxing_status_text(boxing_progress, order);
  raylib::DrawText(
      status_text.c_str(),
      static_cast<int>(ui_constants::pct_to_pixels_x(
          box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f, screen_width)),
      static_cast<int>(ui_constants::pct_to_pixels_y(y, screen_height)),
      instruction_font_size, status_color);
  y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;

  if (boxing_progress.state != BoxingState::PutItems) {
    return;
  }

  raylib::DrawText(
      "Items to place:",
      static_cast<int>(ui_constants::pct_to_pixels_x(
          box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f, screen_width)),
      static_cast<int>(ui_constants::pct_to_pixels_y(y, screen_height)),
      instruction_font_size,
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
  y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;

  std::map<ItemType, int> item_counts = count_items(order.selected_items);
  for (const auto &[item_type, count] : item_counts) {
    std::string item_text =
        "  " + item_type_to_string(item_type) + " x" + std::to_string(count);
    raylib::DrawText(
        item_text.c_str(),
        static_cast<int>(ui_constants::pct_to_pixels_x(
            box_x + ui_constants::CONTENT_PADDING_PCT * 2.0f, screen_width)),
        static_cast<int>(ui_constants::pct_to_pixels_y(y, screen_height)),
        instruction_font_size,
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
    y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
  }
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
                     screen_height);

    const afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    const BoxingProgress &boxing_progress =
        boxing_progress_entity.get<BoxingProgress>();

    float y = header_y + ui_constants::HEADER_FONT_SIZE_PCT +
              ui_constants::HEADER_TO_CONTENT_SPACING_PCT;

    if (!boxing_progress.order_id.has_value()) {
      const afterhours::Entity &queue_entity =
          afterhours::EntityHelper::get_singleton<OrderQueue>();
      const OrderQueue &queue = queue_entity.get<OrderQueue>();
      render_order_selection_list(box_x, y, queue, screen_width, screen_height);
    } else {
      for (const afterhours::Entity &order_entity :
           afterhours::EntityQuery()
               .whereID(boxing_progress.order_id.value())
               .whereHasComponent<Order>()
               .gen()) {
        const Order &order = order_entity.get<Order>();
        render_boxing_progress(box_x, y, boxing_progress, order, screen_width,
                               screen_height);
        break;
      }
    }

    float instruction_y =
        box_y + box_height - ui_constants::INSTRUCTION_PADDING_PCT;
    draw_instruction_text(
        "[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
        box_x + ui_constants::HEADER_PADDING_PCT, instruction_y, screen_width,
        screen_height);
  }
};
