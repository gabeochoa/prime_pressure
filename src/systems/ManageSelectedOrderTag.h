#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ManageSelectedOrderTag : afterhours::System<Order> {
  void for_each_with(afterhours::Entity &order_entity, Order &,
                     float) override {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order =
        selected_order_entity.get<SelectedOrder>();

    afterhours::EntityID order_id = order_entity.id;
    bool should_have_tag = selected_order.order_id.has_value() &&
                           selected_order.order_id.value() == order_id;
    bool has_tag = order_entity.hasTag(GameTag::IsSelectedOrder);

    // TODO add tag toggler
    if (should_have_tag && !has_tag) {
      order_entity.enableTag(GameTag::IsSelectedOrder);
    } else if (!should_have_tag && has_tag) {
      order_entity.disableTag(GameTag::IsSelectedOrder);
    }
  }
};
