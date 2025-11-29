#pragma once

#include "../components.h"
#include <afterhours/ah.h>

template <typename... Components>
struct RenderSystem : afterhours::System<Components...> {
  bool should_run(float) const override { return true; }
};

template <ViewState View> struct ViewRenderSystem : afterhours::System<> {
  bool should_run(float) const override {
    return true;
  }
};

struct ComputerViewRenderSystem : ViewRenderSystem<ViewState::Computer> {};
struct WarehouseViewRenderSystem : ViewRenderSystem<ViewState::Warehouse> {};
struct BoxingViewRenderSystem : ViewRenderSystem<ViewState::Boxing> {};
