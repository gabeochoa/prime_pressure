#pragma once

#include "../components.h"
#include "../game.h"
#include "../render_backend.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>

struct BeginWorldRender : afterhours::System<> {
  virtual void once(float) const override {
    render_backend::BeginTextureMode(mainRT);
    render_backend::ClearBackground(raylib::BLACK);
  }
};

struct EndWorldRender : afterhours::System<> {
  virtual void once(float) const override { render_backend::EndTextureMode(); }
};

struct BeginPostProcessingRender : afterhours::System<> {
  virtual void once(float) const override {
    render_backend::BeginDrawing();
    render_backend::ClearBackground(raylib::BLACK);
  }
};

struct EndDrawing : afterhours::System<> {
  virtual void once(float) const override { render_backend::EndDrawing(); }
};

static void draw_view_box(float x_pct, float y_pct, float width_pct,
                          float height_pct, int screen_width,
                          int screen_height, ViewState current_view,
                          ViewState this_view) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  float width = ui_constants::pct_to_pixels_x(width_pct, screen_width);
  float height = ui_constants::pct_to_pixels_y(height_pct, screen_height);

  raylib::Color border_color = (current_view == this_view)
                                   ? ui_colors::TERMINAL_GREEN
                                   : ui_colors::TERMINAL_GRAY;
  raylib::Color outer_border_color = (current_view == this_view)
                                        ? ui_colors::TERMINAL_DARK_GREEN
                                        : ui_colors::TERMINAL_GRAY;
  
  // Double border effect
  raylib::DrawRectangleLinesEx(raylib::Rectangle{x, y, width, height}, 2.0f, border_color);
  raylib::DrawRectangleLinesEx(raylib::Rectangle{x - 4, y - 4, width + 8, height + 8}, 1.0f, outer_border_color);
}

static void draw_view_header(const char *text, float x_pct, float y_pct,
                             ViewState current_view, ViewState this_view,
                             int screen_width, int screen_height,
                             raylib::Font font = raylib::GetFontDefault()) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  int font_size = ui_constants::pct_to_font_size(
      ui_constants::HEADER_FONT_SIZE_PCT, screen_height);

  raylib::Color text_color = (current_view == this_view) ? ui_colors::TERMINAL_GREEN : ui_colors::TERMINAL_GRAY;

  std::string display_text = "> ";
  if (current_view == this_view) {
    display_text += "[ ";
    display_text += text;
    display_text += " ]";
    
    // Blinking cursor effect for active header
    if (static_cast<int>(raylib::GetTime() * 2) % 2 == 0) {
        display_text += "_";
    }
  } else {
    display_text += text;
  }

  raylib::DrawTextEx(font, display_text.c_str(), raylib::Vector2{x, y},
                     static_cast<float>(font_size), 1.0f, text_color);
}

static void
draw_instruction_text(const char *text, float x_pct, float y_pct,
                      int screen_width, int screen_height,
                      raylib::Font font = raylib::GetFontDefault()) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  int font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  raylib::Color text_color = ui_colors::TERMINAL_GREEN;
  text_color.a = 180; // Slight transparency

  raylib::DrawTextEx(font, text, raylib::Vector2{x, y},
                     static_cast<float>(font_size), 1.0f, text_color);
}

static void draw_blinking_cursor(float x, float y, int font_size, raylib::Color color) {
    if (static_cast<int>(raylib::GetTime() * 2) % 2 == 0) {
        raylib::DrawRectangle(static_cast<int>(x), static_cast<int>(y), 10, font_size, color);
    }
}
