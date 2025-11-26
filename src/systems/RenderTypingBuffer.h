#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderTypingBuffer : RenderSystem<> {
  void once(float) const override {
    const afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = ui_constants::BOX_X_PCT;
    float box_y = ui_constants::TYPING_BUFFER_Y_PCT;
    float box_width = ui_constants::BOX_WIDTH_PCT;
    float box_height = ui_constants::TYPING_BUFFER_HEIGHT_PCT;

    draw_view_box(box_x, box_y, box_width, box_height, screen_width,
                  screen_height);

    float text_x = box_x + ui_constants::HEADER_PADDING_PCT;
    float text_y = box_y + ui_constants::HEADER_PADDING_PCT;
    int font_size = ui_constants::pct_to_font_size(
        ui_constants::HEADER_FONT_SIZE_PCT, screen_height);

    std::string display_text = "KEY: ";
    if (!buffer.buffer.empty()) {
      display_text += buffer.buffer;
    } else {
      display_text += "_";
    }

    raylib::Color text_color =
        buffer.has_error
            ? ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Error)
            : ui_constants::get_theme_color(
                  afterhours::ui::Theme::Usage::Secondary);

    raylib::DrawTextEx(
        uiFont, display_text.c_str(),
        raylib::Vector2{ui_constants::pct_to_pixels_x(text_x, screen_width),
                        ui_constants::pct_to_pixels_y(text_y, screen_height)},
        static_cast<float>(font_size), 1.0f, text_color);
  }
};
