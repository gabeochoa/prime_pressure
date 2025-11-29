#pragma once

#include <afterhours/src/plugins/ui/theme.h>
#include <afterhours/src/plugins/ui/theme_defaults.h>

namespace ui_constants {

constexpr float BOX_X_PCT = 20.0f / 1280.0f;
constexpr float BOX_Y_PCT = 60.0f / 720.0f;
constexpr float BOX_WIDTH_PCT = 1240.0f / 1280.0f;
constexpr float BOX_HEIGHT_PCT = 520.0f / 720.0f;

constexpr float COMPUTER_X_PCT = 200.0f / 1280.0f;
constexpr float COMPUTER_Y_PCT = 20.0f / 720.0f;
constexpr float COMPUTER_WIDTH_PCT = 880.0f / 1280.0f;
constexpr float COMPUTER_HEIGHT_PCT = 200.0f / 720.0f;

constexpr float WAREHOUSE_X_PCT = 20.0f / 1280.0f;
constexpr float WAREHOUSE_Y_PCT = 240.0f / 720.0f;
constexpr float WAREHOUSE_WIDTH_PCT = 620.0f / 1280.0f;
constexpr float WAREHOUSE_HEIGHT_PCT = 440.0f / 720.0f;

constexpr float BOXING_X_PCT = 640.0f / 1280.0f;
constexpr float BOXING_Y_PCT = 240.0f / 720.0f;
constexpr float BOXING_WIDTH_PCT = 620.0f / 1280.0f;
constexpr float BOXING_HEIGHT_PCT = 440.0f / 720.0f;

constexpr float TYPING_BUFFER_Y_PCT = 640.0f / 720.0f;
constexpr float TYPING_BUFFER_HEIGHT_PCT = 60.0f / 720.0f;

constexpr float HEADER_FONT_SIZE_PCT = 24.0f / 720.0f;
constexpr float BODY_FONT_SIZE_PCT = 20.0f / 720.0f;
constexpr float INSTRUCTION_FONT_SIZE_PCT = 16.0f / 720.0f;

constexpr float ORDER_ITEM_SPACING_PCT = 30.0f / 720.0f;
constexpr float HEADER_PADDING_PCT = 10.0f / 720.0f;
constexpr float HEADER_TO_CONTENT_SPACING_PCT = 20.0f / 720.0f;
constexpr float CONTENT_PADDING_PCT = 20.0f / 720.0f;
constexpr float INSTRUCTION_PADDING_PCT = 60.0f / 720.0f;

constexpr float TYPING_BUFFER_TIMEOUT = 5.0f;
constexpr float ORDER_GENERATION_INTERVAL = 1.0f;

constexpr float CONVEYOR_START_X_PCT = 0.005f;
constexpr float CONVEYOR_END_X_PCT = 1.0f;
constexpr float CONVEYOR_Y_PCT = 0.5f;
constexpr float CONVEYOR_SPEED =
    (CONVEYOR_END_X_PCT - CONVEYOR_START_X_PCT) / 10.0f;
constexpr float CONVEYOR_BELT_HEIGHT_PCT = 80.0f / 720.0f;
constexpr float CONVEYOR_ITEM_VERTICAL_SPACING_PCT = 40.0f / 720.0f;

inline float pct_to_pixels_x(float pct, int screen_width) {
  return pct * static_cast<float>(screen_width);
}

inline float pct_to_pixels_y(float pct, int screen_height) {
  return pct * static_cast<float>(screen_height);
}

inline int pct_to_font_size(float pct, int screen_height) {
  return static_cast<int>(pct * static_cast<float>(screen_height));
}

inline afterhours::Color get_theme_color(afterhours::ui::Theme::Usage usage) {
  return afterhours::ui::imm::ThemeDefaults::get().get_theme().from_usage(
      usage);
}

} // namespace ui_constants

namespace ui_colors {
    // Retro Terminal Palette
    const raylib::Color TERMINAL_GREEN = raylib::Color{0, 255, 0, 255};
    const raylib::Color TERMINAL_DARK_GREEN = raylib::Color{0, 100, 0, 255};
    const raylib::Color TERMINAL_BLACK = raylib::Color{10, 10, 10, 255}; // Slightly lighter than pure black
    const raylib::Color TERMINAL_AMBER = raylib::Color{255, 176, 0, 255};
    const raylib::Color TERMINAL_RED = raylib::Color{255, 50, 50, 255};
    const raylib::Color TERMINAL_GRAY = raylib::Color{80, 80, 80, 255};
}
