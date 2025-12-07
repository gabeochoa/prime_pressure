#pragma once

#include "../components.h"
#include <afterhours/ah.h>

struct ManageSelectedOrderTagSystem : afterhours::System<Order> {
private:
  const SelectedOrder *selected_order_ptr = nullptr;

public:
  void once(float) override {
    selected_order_ptr = &get_singleton_as<SelectedOrder>();
  }

  void for_each_with(afterhours::Entity &order_entity, Order &,
                     float) override {
    if (!selected_order_ptr)
      return;

    order_entity.setTag(GameTag::IsSelectedOrder,
                        selected_order_ptr->order_id.order_id.has_value() &&
                            selected_order_ptr->order_id.order_id.value() ==
                                order_entity.id);
  }
};
