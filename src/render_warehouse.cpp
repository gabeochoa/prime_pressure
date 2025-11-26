#include "render_views.h"

#include "systems/RenderWarehouseViewBelt.h"
#include "systems/RenderWarehouseViewBox.h"
#include "systems/RenderWarehouseViewInstructions.h"
#include "systems/RenderWarehouseViewItems.h"
#include "systems/RenderWarehouseViewOrderInfo.h"
#include <afterhours/ah.h>

void register_render_warehouse_systems(afterhours::SystemManager &systems) {
  systems.register_render_system(std::make_unique<RenderWarehouseViewBox>());
  systems.register_render_system(std::make_unique<RenderWarehouseViewBelt>());
  systems.register_render_system(
      std::make_unique<RenderWarehouseViewOrderInfo>());
  systems.register_render_system(std::make_unique<RenderWarehouseViewItems>());
  systems.register_render_system(
      std::make_unique<RenderWarehouseViewInstructions>());
}
