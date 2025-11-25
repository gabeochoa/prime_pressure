#pragma once

#include "../components.h"
#include "../log.h"
#include <afterhours/ah.h>

struct GrabItem
    : afterhours::System<Item, afterhours::tags::All<GameTag::IsGrabbed>> {
  void for_each_with(afterhours::Entity &entity, Item &item, float) override {
    log_info("GrabItem: Entity {} grabbed item '{}'", entity.id, item.name());
  }
};
