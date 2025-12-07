#pragma once

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"
#include <afterhours/ah.h>

struct RenderWarehouseViewBox : WarehouseViewRenderSystem {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;
    float screen_width_f = static_cast<float>(screen_width);
    float screen_height_f = static_cast<float>(screen_height);

    TriLayout layout = compute_tri_layout(screen_width, screen_height);

    float box_x = layout.warehouse.x / screen_width_f;
    float box_y = layout.warehouse.y / screen_height_f;

    PixelRect px{layout.warehouse.x, layout.warehouse.y, layout.warehouse.w,
                 layout.warehouse.h};
    raylib::DrawRectangleGradientV(
        static_cast<int>(px.x), static_cast<int>(px.y), static_cast<int>(px.w),
        static_cast<int>(px.h), raylib::Color{152, 128, 104, 235},
        raylib::Color{116, 94, 76, 235});

    draw_view_box(layout.warehouse, active_view.current_view,
                  ViewState::Warehouse);

    float header_x = box_x + ui_constants::HEADER_PADDING_PCT;
    float header_y = box_y + ui_constants::HEADER_PADDING_PCT;
    draw_view_header("WAREHOUSE SCREEN", header_x, header_y,
                     active_view.current_view, ViewState::Warehouse,
                     screen_width, screen_height, uiFont);

    float drop_w = px.w * 0.38f;
    float drop_h = px.h * 0.3f;
    float drop_x = px.x + (px.w - drop_w) * 0.5f;
    float drop_y = px.y + (px.h - drop_h) * 0.5f;
    draw_dashed_rect(PixelRect{drop_x, drop_y, drop_w, drop_h},
                     raylib::Color{200, 170, 120, 220}, 10.0f, 6.0f);
  }
};
