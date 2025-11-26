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
                          int screen_height) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  float width = ui_constants::pct_to_pixels_x(width_pct, screen_width);
  float height = ui_constants::pct_to_pixels_y(height_pct, screen_height);

  raylib::Color border_color =
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Primary);
  raylib::DrawRectangleLines(static_cast<int>(x), static_cast<int>(y),
                             static_cast<int>(width), static_cast<int>(height),
                             border_color);
}

static void draw_view_header(const char *text, float x_pct, float y_pct,
                             ViewState current_view, ViewState this_view,
                             int screen_width, int screen_height) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  int font_size = ui_constants::pct_to_font_size(
      ui_constants::HEADER_FONT_SIZE_PCT, screen_height);

  raylib::Color text_color = ui_constants::get_theme_color(
      current_view == this_view ? afterhours::ui::Theme::Usage::Accent
                                : afterhours::ui::Theme::Usage::Primary);

  std::string display_text = "> ";
  if (current_view == this_view) {
    display_text += "[";
    display_text += text;
    display_text += "]";
  } else {
    display_text += text;
  }

  raylib::DrawText(display_text.c_str(), static_cast<int>(x),
                   static_cast<int>(y), font_size, text_color);
}

static void draw_instruction_text(const char *text, float x_pct, float y_pct,
                                  int screen_width, int screen_height) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  int font_size = ui_constants::pct_to_font_size(
      ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

  raylib::Color text_color =
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
  text_color = raylib::Color{text_color.r, text_color.g, text_color.b, 128};

  raylib::DrawText(text, static_cast<int>(x), static_cast<int>(y), font_size,
                   text_color);
}
