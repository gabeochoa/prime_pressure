#pragma once

#include "../components.h"
#include <afterhours/ah.h>
#include <magic_enum/magic_enum.hpp>

struct SpawnItems : afterhours::System<> {
  virtual void once(float) override {
    static bool initialized = false;
    if (initialized) {
      return;
    }
    initialized = true;

    const auto items = magic_enum::enum_values<ItemType>();

    for (ItemType item_type : items) {
      afterhours::Entity &item_entity =
          afterhours::EntityHelper::createEntity();
      Item &item = item_entity.addComponent<Item>();
      item.type = item_type;
      item_entity.enableTag(GameTag::IsOnShelf);
    }
  }
};
