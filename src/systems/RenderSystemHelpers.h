#pragma once

#include "../game.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

using namespace afterhours;

struct BeginWorldRender : System<> {
  virtual void once(float) const override {
    render_backend::BeginTextureMode(mainRT);
    render_backend::ClearBackground(raylib::DARKGRAY);
  }
};

struct EndWorldRender : System<> {
  virtual void once(float) const override { render_backend::EndTextureMode(); }
};

struct EndDrawing : System<> {
  virtual void once(float) const override { render_backend::EndDrawing(); }
};
