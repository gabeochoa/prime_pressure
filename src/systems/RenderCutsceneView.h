#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>
#include <string>

struct RenderCutsceneView : RenderSystem<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Cutscene;
  }

  void once(float) const override {
    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    raylib::DrawRectangleGradientV(0, 0, screen_width, screen_height,
                                   raylib::Color{22, 32, 52, 255},
                                   raylib::Color{10, 14, 24, 255});

    float margin = 64.0f;
    raylib::Rectangle window{margin, margin,
                             static_cast<float>(screen_width) - margin * 2.0f,
                             static_cast<float>(screen_height) - margin * 2.0f};
    raylib::DrawRectangleRounded(window, 0.06f, 10,
                                 raylib::Color{242, 245, 252, 240});

    float title_h = 40.0f;
    raylib::Rectangle title_bar{window.x, window.y, window.width, title_h};
    raylib::DrawRectangleRounded(title_bar, 0.06f, 10,
                                 ui_colors::TASKBAR_ACCENT);

    int header_font = ui_constants::pct_to_font_size(
        ui_constants::HEADER_FONT_SIZE_PCT, screen_height);
    raylib::DrawTextEx(
        uiFont, "XP MAIL", {title_bar.x + 18.0f, title_bar.y + 8.0f},
        static_cast<float>(header_font), 1.0f, ui_colors::TASKBAR_TEXT);

    float body_pad = 24.0f;
    raylib::Rectangle body{window.x + body_pad, window.y + title_h + body_pad,
                           window.width - body_pad * 2.0f,
                           window.height - title_h - body_pad * 2.0f};
    raylib::DrawRectangleRounded(body, 0.04f, 10,
                                 raylib::Color{252, 253, 255, 250});
    raylib::DrawRectangleRoundedLines(body, 0.04f, 10,
                                      raylib::Color{200, 208, 225, 255});

    int body_font = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    float text_y = body.y + 12.0f;

    std::string subject = "Subject: Late-shift briefing";
    raylib::DrawTextEx(uiFont, subject.c_str(), {body.x + 12.0f, text_y},
                       static_cast<float>(body_font), 1.0f,
                       raylib::Color{40, 56, 86, 255});
    text_y += static_cast<float>(body_font) + 12.0f;

    std::string lines[] = {
        "HQ: The midnight route is live. Orders will stream faster than usual.",
        "Double-check packing slips. If a popup asks, \"Accept reroute?\", hit",
        "\"Y\" to keep the conveyor moving. Keep an eye on the XP taskbar--new",
        "messages land there first."};

    for (const std::string &line : lines) {
      raylib::DrawTextEx(uiFont, line.c_str(), {body.x + 12.0f, text_y},
                         static_cast<float>(body_font), 1.0f,
                         raylib::Color{50, 64, 92, 255});
      text_y += static_cast<float>(body_font) + 8.0f;
    }

    std::string hint = "Press Enter to continue";
    int hint_font = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);
    raylib::Vector2 hint_size = raylib::MeasureTextEx(
        uiFont, hint.c_str(), static_cast<float>(hint_font), 1.0f);
    raylib::Vector2 hint_pos{window.x + window.width - hint_size.x - 24.0f,
                             window.y + window.height - hint_size.y - 16.0f};
    draw_badge(hint_pos, hint, hint_font, ui_colors::TASKBAR_ACCENT,
               ui_colors::TASKBAR_TEXT, 10.0f);
  }
};
