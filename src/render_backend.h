#pragma once

#include "rl.h"

namespace render_backend {
bool is_headless_mode = false;

inline void BeginDrawing() {
  if (!is_headless_mode)
    raylib::BeginDrawing();
}

inline void EndDrawing() {
  if (!is_headless_mode)
    raylib::EndDrawing();
}

inline void BeginTextureMode(raylib::RenderTexture2D target) {
  if (!is_headless_mode)
    raylib::BeginTextureMode(target);
}

inline void EndTextureMode() {
  if (!is_headless_mode)
    raylib::EndTextureMode();
}

inline void ClearBackground(raylib::Color color) {
  if (!is_headless_mode)
    raylib::ClearBackground(color);
}
} // namespace render_backend
