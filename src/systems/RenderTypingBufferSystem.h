#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderTypingBufferSystem : RenderSystem<> {
  bool should_run(float) const override { return true; }

  static raylib::Color status_bg(const TypingBuffer &buffer, bool primary) {
    raylib::Color base = ui_colors::TASKBAR_BASE;
    if (buffer.status == TypingStatus::Match) {
      base = ui_colors::TASKBAR_GOOD;
    } else if (buffer.status == TypingStatus::Error) {
      base = ui_colors::TASKBAR_BAD;
    }
    if (!primary) {
      base.a = static_cast<unsigned char>(static_cast<float>(base.a) * 0.9f);
    }
    return base;
  }

  static raylib::Color status_fg(const TypingBuffer &buffer) {
    if (buffer.status == TypingStatus::Error) {
      return ui_colors::TERMINAL_RED;
    }
    if (buffer.status == TypingStatus::Match) {
      return ui_colors::TERMINAL_GREEN;
    }
    return raylib::Color{40, 50, 70, 255}; // Windows XP body text color
  }

  static std::string status_label(const TypingBuffer &buffer) {
    if (buffer.status == TypingStatus::Match) {
      return "match";
    }
    if (buffer.status == TypingStatus::Error) {
      return "error";
    }
    if (buffer.status == TypingStatus::Typing) {
      return "typing";
    }
    return "ready";
  }

  static std::string hint_text() {
    return "TAB: cycle active order   ENTER: clear";
  }

  static void draw_primary_bar(const LayoutRect &band,
                               const TypingBuffer &buffer, int screen_width,
                               int screen_height) {
    (void)screen_width;
    float bar_h = ui_constants::pct_to_pixels_y(
        ui_constants::TASKBAR_HEIGHT_PCT, screen_height);
    float bar_y = band.y + band.h - bar_h;
    raylib::Rectangle rect{band.x, bar_y, band.w, bar_h};

    raylib::Color bg = status_bg(buffer, true);
    raylib::DrawRectangleRounded(rect, 0.06f, 8, bg);
    raylib::DrawRectangleRoundedLines(rect, 0.06f, 8,
                                      ui_colors::TASKBAR_ACCENT);

    float pad = ui_constants::pct_to_pixels_y(ui_constants::TASKBAR_PADDING_PCT,
                                              screen_height);
    int line_font = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int hint_font = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    raylib::Vector2 line_pos{rect.x + pad, rect.y + pad};
    raylib::Color fg = status_fg(buffer);

    std::string status_block = "STATUS: " + status_label(buffer);
    raylib::DrawTextEx(uiFont, status_block.c_str(), line_pos,
                       static_cast<float>(line_font), 1.0f, fg);

    raylib::Vector2 hint_pos{
        rect.x + pad, line_pos.y + static_cast<float>(line_font) + pad * 0.5f};
    raylib::DrawTextEx(uiFont, hint_text().c_str(), hint_pos,
                       static_cast<float>(hint_font), 1.0f,
                       ui_colors::TASKBAR_TEXT);
  }

  static void draw_hint_strip(const LayoutRect &rect,
                              const TypingBuffer &buffer, int screen_width,
                              int screen_height) {
    (void)screen_width;
    float pad = ui_constants::pct_to_pixels_y(
        ui_constants::HINT_STRIP_PADDING_PCT, screen_height);
    float h = ui_constants::pct_to_pixels_y(ui_constants::HINT_STRIP_HEIGHT_PCT,
                                            screen_height);
    raylib::Rectangle bar{rect.x + pad, rect.y + rect.h - h - pad,
                          rect.w - pad * 2.0f, h};

    raylib::Color bg = status_bg(buffer, false);
    raylib::DrawRectangleRounded(bar, 0.08f, 6, bg);

    int font = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);
    std::string text = "TAB: cycle order   ENTER: clear";
    raylib::DrawTextEx(uiFont, text.c_str(),
                       raylib::Vector2{bar.x + pad, bar.y + pad * 0.25f},
                       static_cast<float>(font), 1.0f, status_fg(buffer));
  }

  void once(float) const override {
    const afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    TriLayout layout = compute_tri_layout(screen_width, screen_height);

    if (active_view.current_view == ViewState::Warehouse ||
        active_view.current_view == ViewState::Boxing) {
      const LayoutRect &rect = active_view.current_view == ViewState::Warehouse
                                   ? layout.warehouse
                                   : layout.boxing;
      draw_hint_strip(rect, buffer, screen_width, screen_height);
    }
  }
};
