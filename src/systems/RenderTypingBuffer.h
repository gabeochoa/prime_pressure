#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderTypingBuffer : afterhours::System<> {
  void once(float) const override {
    const afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();

    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = 20.0f;
    float box_y = static_cast<float>(screen_height) - 80.0f;
    float box_width = static_cast<float>(screen_width) - 40.0f;
    float box_height = 60.0f;

    raylib::DrawRectangleLines(static_cast<int>(box_x), static_cast<int>(box_y),
                               static_cast<int>(box_width),
                               static_cast<int>(box_height), raylib::GREEN);

    std::string display_text = "TYPING: " + buffer.buffer;
    raylib::DrawText(display_text.c_str(), static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + 15), 24, raylib::YELLOW);
  }
};
