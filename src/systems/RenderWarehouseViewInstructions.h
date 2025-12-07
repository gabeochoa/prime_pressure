#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderWarehouseViewInstructions : WarehouseViewRenderSystem {
  void once(float) const override {

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;
    float screen_width_f = static_cast<float>(screen_width);
    float screen_height_f = static_cast<float>(screen_height);

    TriLayout layout = compute_tri_layout(screen_width, screen_height);

    float box_x = layout.warehouse.x / screen_width_f;
    float box_y = layout.warehouse.y / screen_height_f;
    float box_height = layout.warehouse.h / screen_height_f;

    float instruction_y =
        box_y + box_height - ui_constants::INSTRUCTION_PADDING_PCT;
    int font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);
    raylib::DrawTextEx(
        uiFont, "[Type item names to grab items]",
        raylib::Vector2{ui_constants::pct_to_pixels_x(
                            box_x + ui_constants::HEADER_PADDING_PCT,
                            screen_width),
                        ui_constants::pct_to_pixels_y(instruction_y,
                                                      screen_height)},
        static_cast<float>(font_size), 1.0f, raylib::Color{90, 90, 90, 255});
  }
};
