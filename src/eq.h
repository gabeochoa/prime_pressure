#pragma once

#include "components.h"
#include <afterhours/ah.h>

struct EQ : afterhours::EntityQuery<EQ> {
  struct WhereHasOrderID : Modification {
    afterhours::EntityID order_id;
    explicit WhereHasOrderID(afterhours::EntityID id) : order_id(id) {}
    bool operator()(const afterhours::Entity &entity) const override {
      if (!entity.has<ConveyorItem>()) {
        return false;
      }
      const ConveyorItem &item = entity.get<ConveyorItem>();
      return item.order_id == order_id;
    }
  };

  EQ &whereHasOrderID(afterhours::EntityID order_id) {
    return add_mod(new WhereHasOrderID(order_id));
  }
};
