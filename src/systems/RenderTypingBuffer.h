#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

struct RenderTypingBuffer : afterhours::System<> {
  void once(float) const override {
    const afterhours::Entity &buffer_entity =
        afterhours::EntityHelper::get_singleton<TypingBuffer>();
    const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();

    // Use render texture dimensions (we're drawing to mainRT)
    int render_height = mainRT.texture.height;
    std::string display_text = "Typing: " + buffer.buffer;
    raylib::DrawText(display_text.c_str(), 20, render_height - 60, 30,
                     raylib::YELLOW);

    raylib::DrawText("PRIME PRESSURE", 20, 20, 40, raylib::WHITE);
  }
};
