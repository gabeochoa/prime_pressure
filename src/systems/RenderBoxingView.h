#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderBoxingView : afterhours::System<> {
  void once(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();

    if (active_view.current_view != ViewState::Boxing) {
      return;
    }

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = 20.0f;
    float box_y = 60.0f;
    float box_width = static_cast<float>(screen_width) - 40.0f;
    float box_height = static_cast<float>(screen_height) - 200.0f;

    raylib::DrawRectangleLines(static_cast<int>(box_x), static_cast<int>(box_y),
                               static_cast<int>(box_width),
                               static_cast<int>(box_height), raylib::GREEN);

    raylib::DrawText("> BOXING SCREEN", static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + 10), 24, raylib::GREEN);

    float center_x = box_x + box_width / 2.0f;
    float center_y = box_y + box_height / 2.0f;
    std::string coming_soon = "Coming soon";
    int text_width = raylib::MeasureText(coming_soon.c_str(), 32);
    raylib::DrawText(coming_soon.c_str(),
                     static_cast<int>(center_x - text_width / 2),
                     static_cast<int>(center_y - 16), 32, raylib::GRAY);

    raylib::DrawText("[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
                     static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + box_height - 60), 16,
                     raylib::GRAY);
  }
};
