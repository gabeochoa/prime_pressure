#include "render_views.h"

#include "systems/RenderBoxingViewSystem.h"
#include <afterhours/ah.h>

void register_render_boxing_systems(afterhours::SystemManager &systems) {
  systems.register_render_system(std::make_unique<RenderBoxingViewSystem>());
}
