#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderWarehouseViewBelt : WarehouseViewRenderSystem {
  void once(float) const override {
    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;
    float screen_width_f = static_cast<float>(screen_width);
    float screen_height_f = static_cast<float>(screen_height);

    TriLayout layout = compute_tri_layout(screen_width, screen_height);

    float box_x = layout.warehouse.x / screen_width_f;
    float box_y = layout.warehouse.y / screen_height_f;
    float box_width = layout.warehouse.w / screen_width_f;
    float box_height = layout.warehouse.h / screen_height_f;

    PixelRect belt_rect{
        ui_constants::pct_to_pixels_x(box_x, screen_width),
        ui_constants::pct_to_pixels_y(box_y, screen_height),
        ui_constants::pct_to_pixels_x(box_width, screen_width),
        ui_constants::pct_to_pixels_y(box_height, screen_height)};

    float belt_y_pct = box_y + box_height * ui_constants::CONVEYOR_Y_PCT;
    float belt_y = ui_constants::pct_to_pixels_y(belt_y_pct, screen_height);
    float belt_height = ui_constants::pct_to_pixels_y(
        ui_constants::CONVEYOR_BELT_HEIGHT_PCT, screen_height);
    float belt_start_x = ui_constants::pct_to_pixels_x(box_x, screen_width);
    float belt_end_x =
        ui_constants::pct_to_pixels_x(box_x + box_width, screen_width);
    float belt_width = belt_end_x - belt_start_x;

    draw_stripes(belt_rect, raylib::Color{18, 22, 28, 255},
                 raylib::Color{80, 180, 220, 160}, 28.0f, 18.0f);
    raylib::DrawRectangle(static_cast<int>(belt_start_x),
                          static_cast<int>(belt_y - belt_height / 2.0f),
                          static_cast<int>(belt_width),
                          static_cast<int>(belt_height),
                          raylib::Color{12, 16, 20, 220});
  }
};
