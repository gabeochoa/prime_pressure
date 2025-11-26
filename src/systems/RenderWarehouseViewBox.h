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

    if (active_view.current_view != ViewState::Warehouse) {
      return;
    }

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = ui_constants::BOX_X_PCT;
    float box_y = ui_constants::BOX_Y_PCT;
    float box_width = ui_constants::BOX_WIDTH_PCT;
    float box_height = ui_constants::BOX_HEIGHT_PCT;

    draw_view_box(box_x, box_y, box_width, box_height, screen_width,
                  screen_height);

    float header_x = box_x + ui_constants::HEADER_PADDING_PCT;
    float header_y = box_y + ui_constants::HEADER_PADDING_PCT;
    draw_view_header("WAREHOUSE SCREEN", header_x, header_y,
                     active_view.current_view, ViewState::Warehouse,
                     screen_width, screen_height, uiFont);
  }
};
