#pragma once

#include "../components.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>

struct ManageConveyorItems : afterhours::System<ConveyorItem> {
  void for_each_with(afterhours::Entity &entity, ConveyorItem &item,
                     float dt) override {
    if (!item.is_moving) {
      return;
    }

    item.x_position += item.speed * dt;

    if (item.x_position >= ui_constants::CONVEYOR_END_X_PCT) {
      entity.cleanup = true;
    }
  }
};
