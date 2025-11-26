#pragma once

#include "../components.h"
#include <afterhours/ah.h>

template <typename... Components>
struct RenderSystem : afterhours::System<Components...> {
  bool should_run(float) const override { return true; }
};

template <ViewState View> struct ViewRenderSystem : afterhours::System<> {
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == View;
  }
};

struct ComputerViewRenderSystem : ViewRenderSystem<ViewState::Computer> {};
struct WarehouseViewRenderSystem : ViewRenderSystem<ViewState::Warehouse> {};
struct BoxingViewRenderSystem : ViewRenderSystem<ViewState::Boxing> {};
