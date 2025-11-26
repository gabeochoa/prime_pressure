#include "render_views.h"

#include "systems/RenderComputerView.h"
#include <afterhours/ah.h>

void register_render_computer_systems(afterhours::SystemManager &systems) {
  systems.register_render_system(std::make_unique<RenderComputerView>());
}
