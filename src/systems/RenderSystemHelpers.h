#pragma once

#include "../components.h"
#include "../game.h"
#include "../render_backend.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>
#include <algorithm>

struct BeginWorldRenderSystem : afterhours::System<> {
  virtual void once(float) const override {
    render_backend::BeginTextureMode(mainRT);
    render_backend::ClearBackground(raylib::BLACK);
  }
};

struct EndWorldRenderSystem : afterhours::System<> {
  virtual void once(float) const override { render_backend::EndTextureMode(); }
};

struct BeginPostProcessingRenderSystem : afterhours::System<> {
  virtual void once(float) const override {
    render_backend::BeginDrawing();
    render_backend::ClearBackground(raylib::BLACK);
  }
};

struct EndDrawingSystem : afterhours::System<> {
  virtual void once(float) const override { render_backend::EndDrawing(); }
};

static void draw_view_box(float x_pct, float y_pct, float width_pct,
                          float height_pct, int screen_width, int screen_height,
                          ViewState current_view, ViewState this_view) {
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
  raylib::DrawRectangleLinesEx(raylib::Rectangle{x, y, width, height}, 2.0f,
                               border_color);
  raylib::DrawRectangleLinesEx(
      raylib::Rectangle{x - 4, y - 4, width + 8, height + 8}, 1.0f,
      outer_border_color);
}

struct LayoutRect {
  float x;
  float y;
  float w;
  float h;
};

struct TriLayout {
  LayoutRect computer;
  LayoutRect warehouse;
  LayoutRect boxing;
};

struct PixelRect {
  float x;
  float y;
  float w;
  float h;
};

inline PixelRect to_pixels(const LayoutRect &rect) {
  return PixelRect{rect.x, rect.y, rect.w, rect.h};
}

inline void draw_stripes(const PixelRect &rect, raylib::Color base,
                         raylib::Color stripe, float stripe_width, float gap) {
  raylib::DrawRectangle(static_cast<int>(rect.x), static_cast<int>(rect.y),
                        static_cast<int>(rect.w), static_cast<int>(rect.h),
                        base);
  float x = rect.x - rect.h;
  while (x < rect.x + rect.w) {
    float x0 = x;
    float x1 = x + stripe_width;
    raylib::DrawTriangle(raylib::Vector2{x0, rect.y},
                         raylib::Vector2{x1, rect.y},
                         raylib::Vector2{x0 + rect.h, rect.y + rect.h}, stripe);
    raylib::DrawTriangle(raylib::Vector2{x1, rect.y},
                         raylib::Vector2{x1 + rect.h, rect.y + rect.h},
                         raylib::Vector2{x0 + rect.h, rect.y + rect.h}, stripe);
    x += stripe_width + gap;
  }
}

inline void draw_dashed_rect(const PixelRect &rect, raylib::Color color,
                             float dash_len = 8.0f, float gap = 6.0f) {
  auto horiz = [&](float y) {
    float x = rect.x;
    while (x < rect.x + rect.w) {
      float end = std::min(x + dash_len, rect.x + rect.w);
      raylib::DrawLineEx(raylib::Vector2{x, y}, raylib::Vector2{end, y}, 2.0f,
                         color);
      x += dash_len + gap;
    }
  };
  auto vert = [&](float x) {
    float y = rect.y;
    while (y < rect.y + rect.h) {
      float end = std::min(y + dash_len, rect.y + rect.h);
      raylib::DrawLineEx(raylib::Vector2{x, y}, raylib::Vector2{x, end}, 2.0f,
                         color);
      y += dash_len + gap;
    }
  };
  horiz(rect.y);
  horiz(rect.y + rect.h);
  vert(rect.x);
  vert(rect.x + rect.w);
}

inline void draw_badge(const raylib::Vector2 &pos, const std::string &text,
                       int font_size, raylib::Color bg, raylib::Color fg,
                       float padding = 6.0f) {
  raylib::Vector2 size =
      raylib::MeasureTextEx(raylib::GetFontDefault(), text.c_str(),
                            static_cast<float>(font_size), 1.0f);
  float w = size.x + padding * 2.0f;
  float h = size.y + padding * 1.5f;
  raylib::DrawRectangleRounded(raylib::Rectangle{pos.x, pos.y, w, h}, 0.4f, 6,
                               bg);
  raylib::DrawTextEx(raylib::GetFontDefault(), text.c_str(),
                     raylib::Vector2{pos.x + padding, pos.y + padding * 0.5f},
                     static_cast<float>(font_size), 1.0f, fg);
}

inline TriLayout compute_tri_layout(int screen_width, int screen_height) {
  // Fixed layout showing all views simultaneously
  float computer_h = ui_constants::COMPUTER_BAND_ACTIVE_PCT *
                     static_cast<float>(screen_height);
  float bottom_h = static_cast<float>(screen_height) - computer_h;

  float warehouse_w =
      ui_constants::WAREHOUSE_WIDTH_PCT_TRI * static_cast<float>(screen_width);
  float boxing_w =
      ui_constants::BOXING_WIDTH_PCT_TRI * static_cast<float>(screen_width);
  float gap = ui_constants::TRI_GAP_PCT * static_cast<float>(screen_width);

  float total_w = warehouse_w + gap + boxing_w;
  float left_margin =
      std::max(0.0f, (static_cast<float>(screen_width) - total_w) * 0.5f);

  LayoutRect computer{0.0f, 0.0f, static_cast<float>(screen_width), computer_h};
  LayoutRect warehouse{left_margin, computer_h, warehouse_w, bottom_h};
  LayoutRect boxing{left_margin + warehouse_w + gap, computer_h, boxing_w,
                    bottom_h};

  return TriLayout{computer, warehouse, boxing};
}

static void draw_view_box(const LayoutRect &rect, ViewState current_view,
                          ViewState this_view) {
  raylib::Color border_color = (current_view == this_view)
                                   ? raylib::Color{49, 106, 197, 255}
                                   // Windows XP active accent
                                   : ui_colors::TERMINAL_GRAY;
  raylib::Color outer_border_color = (current_view == this_view)
                                         ? raylib::Color{10, 36, 106, 255}
                                         // Windows XP dark accent
                                         : ui_colors::TERMINAL_GRAY;

  raylib::DrawRectangleLinesEx(
      raylib::Rectangle{rect.x, rect.y, rect.w, rect.h}, 2.0f, border_color);
  raylib::DrawRectangleLinesEx(
      raylib::Rectangle{rect.x - 4, rect.y - 4, rect.w + 8, rect.h + 8}, 1.0f,
      outer_border_color);
}

static void draw_view_header(const char *text, float x_pct, float y_pct,
                             ViewState current_view, ViewState this_view,
                             int screen_width, int screen_height,
                             raylib::Font font = raylib::GetFontDefault()) {
  float x = ui_constants::pct_to_pixels_x(x_pct, screen_width);
  float y = ui_constants::pct_to_pixels_y(y_pct, screen_height);
  int font_size = ui_constants::pct_to_font_size(
      ui_constants::HEADER_FONT_SIZE_PCT, screen_height);

  raylib::Color text_color = (current_view == this_view)
                                 ? raylib::Color{40, 50, 70, 255}
                                 // Windows XP body text
                                 : ui_colors::TERMINAL_GRAY;

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

  raylib::Color text_color =
      raylib::Color{70, 70, 70, 255}; // Windows XP sub text
  text_color.a = 180;                 // Slight transparency

  raylib::DrawTextEx(font, text, raylib::Vector2{x, y},
                     static_cast<float>(font_size), 1.0f, text_color);
}

static void draw_keycap(const raylib::Vector2 &pos, const std::string &text,
                        int font_size, raylib::Color bg, raylib::Color fg,
                        raylib::Font font = raylib::GetFontDefault(),
                        float padding = 6.0f) {
  raylib::Vector2 size =
      raylib::MeasureTextEx(font, text.c_str(),
                            static_cast<float>(font_size), 1.0f);
  float w = size.x + padding * 2.0f;
  float h = size.y + padding * 1.2f;
  raylib::DrawRectangleRounded(
      raylib::Rectangle{pos.x, pos.y, w, h}, 0.35f, 6, bg);
  raylib::DrawRectangleRoundedLines(
      raylib::Rectangle{pos.x, pos.y, w, h}, 0.35f, 6,
      raylib::Color{0, 0, 0, 100});
  raylib::DrawTextEx(
      font, text.c_str(),
      raylib::Vector2{pos.x + padding, pos.y + padding * 0.35f},
      static_cast<float>(font_size), 1.0f, fg);
}

static void draw_blinking_cursor(float x, float y, int font_size,
                                 raylib::Color color) {
  if (static_cast<int>(raylib::GetTime() * 2) % 2 == 0) {
    raylib::DrawRectangle(static_cast<int>(x), static_cast<int>(y), 10,
                          font_size, color);
  }
}
