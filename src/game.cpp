#include "game.h"

#include "input_mapping.h"
#include "preload.h"
#include "render_backend.h"
#include "settings.h"
#include "systems/RenderRenderTexture.h"
#include "systems/RenderSystemHelpers.h"
#include "systems/UpdateRenderTexture.h"

#include <afterhours/src/plugins/animation.h>

bool running = true;
raylib::RenderTexture2D mainRT;
raylib::RenderTexture2D screenRT;

using namespace afterhours;

void game() {
  mainRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                     Settings::get().get_screen_height());
  screenRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                       Settings::get().get_screen_height());

  SystemManager systems;

  {
    window_manager::enforce_singletons(systems);
    ui::enforce_singletons<InputAction>(systems);
    input::enforce_singletons(systems);
  }

  {
    input::register_update_systems(systems);
    window_manager::register_update_systems(systems);
  }

  {
    systems.register_update_system(std::make_unique<UpdateRenderTexture>());

    systems.register_render_system(std::make_unique<BeginWorldRender>());
    systems.register_render_system(std::make_unique<EndWorldRender>());
    systems.register_render_system(std::make_unique<RenderRenderTexture>());
    ui::register_render_systems<InputAction>(systems,
                                             InputAction::ToggleUILayoutDebug);
    systems.register_render_system(std::make_unique<EndDrawing>());
  }

  while (running && !raylib::WindowShouldClose()) {
    float dt = raylib::GetFrameTime();
    systems.run(dt);
  }
}
