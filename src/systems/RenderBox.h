#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderBox
    : afterhours::System<Box, afterhours::tags::All<GameTag::IsBox>> {
  void for_each_with(const afterhours::Entity &entity, const Box &box,
                     float) const override {
    (void)entity;
    int render_width = mainRT.texture.width;
    int render_height = mainRT.texture.height;

    float box_x = render_width - 200.0f;
    float box_y = render_height - 150.0f;
    float box_width = 180.0f;
    float box_height = 130.0f;

    raylib::DrawRectangleLines(static_cast<int>(box_x), static_cast<int>(box_y),
                               static_cast<int>(box_width),
                               static_cast<int>(box_height), raylib::WHITE);

    const Box &box_ref = box;
    std::string box_text = "Box: " + std::to_string(box_ref.items.size()) +
                           "/" + std::to_string(box_ref.capacity);
    raylib::DrawText(box_text.c_str(), static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + 10), 20, raylib::WHITE);
  }
};
