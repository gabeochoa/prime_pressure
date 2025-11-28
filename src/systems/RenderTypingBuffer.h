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
    } 

    raylib::Color text_color =
        buffer.has_error
            ? ui_colors::TERMINAL_RED
            : ui_colors::TERMINAL_AMBER;

    raylib::Vector2 text_pos = {ui_constants::pct_to_pixels_x(text_x, screen_width),
                                ui_constants::pct_to_pixels_y(text_y, screen_height)};
                                
    raylib::DrawTextEx(
        uiFont, display_text.c_str(),
        text_pos,
        static_cast<float>(font_size), 1.0f, text_color);
        
    // Blinking cursor
    float text_width = raylib::MeasureTextEx(uiFont, display_text.c_str(), static_cast<float>(font_size), 1.0f).x;
    draw_blinking_cursor(text_pos.x + text_width + 2, text_pos.y, font_size, text_color);
  }
};
