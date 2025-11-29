#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include <afterhours/ah.h>

struct RenderWarehouseViewBelt : WarehouseViewRenderSystem {
  void once(float) const override {
    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = ui_constants::WAREHOUSE_X_PCT;
    float box_y = ui_constants::WAREHOUSE_Y_PCT;
    float box_width = ui_constants::WAREHOUSE_WIDTH_PCT;
    float box_height = ui_constants::WAREHOUSE_HEIGHT_PCT;

    float belt_y_pct = box_y + box_height * ui_constants::CONVEYOR_Y_PCT;
    float belt_y = ui_constants::pct_to_pixels_y(belt_y_pct, screen_height);
    float belt_height = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_BELT_HEIGHT_PCT, screen_height);
    float belt_start_x = ui_constants::pct_to_pixels_x(box_x, screen_width);
    float belt_end_x =
        ui_constants::pct_to_pixels_x(box_x + box_width, screen_width);
    float belt_width = belt_end_x - belt_start_x;

    raylib::Color belt_color =
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Background);
    belt_color = raylib::Color{belt_color.r, belt_color.g, belt_color.b, 200};
    raylib::DrawRectangle(static_cast<int>(belt_start_x),
                          static_cast<int>(belt_y - belt_height / 2.0f),
                          static_cast<int>(belt_width),
                          static_cast<int>(belt_height), belt_color);
  }
};
