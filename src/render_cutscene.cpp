#include "render_views.h"

#include "systems/RenderCutsceneView.h"
#include <afterhours/ah.h>

void register_render_cutscene_systems(afterhours::SystemManager &systems) {
  systems.register_render_system(std::make_unique<RenderCutsceneView>());
}
