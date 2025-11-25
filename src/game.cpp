#include "game.h"

#include "components.h"
#include "input_mapping.h"
#include "log.h"
#include "preload.h"
#include "render_backend.h"
#include "settings.h"
#include "systems/BoxItem.h"
#include "systems/GenerateOrders.h"
#include "systems/GrabItem.h"
#include "systems/MatchItemToOrder.h"
#include "systems/ProcessTypingInput.h"
#include "systems/RenderBox.h"
#include "systems/RenderOrders.h"
#include "systems/RenderRenderTexture.h"
#include "systems/RenderSystemHelpers.h"
#include "systems/RenderTypingBuffer.h"
#include "systems/SpawnItems.h"
#include "systems/UpdateRenderTexture.h"

#include <afterhours/src/plugins/animation.h>

bool running = true;
raylib::RenderTexture2D mainRT;
raylib::RenderTexture2D screenRT;

void game() {
  mainRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                     Settings::get().get_screen_height());
  screenRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                       Settings::get().get_screen_height());

  afterhours::SystemManager systems;

  {
    afterhours::window_manager::enforce_singletons(systems);
    afterhours::ui::enforce_singletons<InputAction>(systems);
    afterhours::input::enforce_singletons(systems);
  }

  {
    // Create singleton entities for TypingBuffer and OrderQueue
    afterhours::Entity &typing_buffer_entity =
        afterhours::EntityHelper::createEntity();
    typing_buffer_entity.addComponent<TypingBuffer>();
    afterhours::EntityHelper::registerSingleton<TypingBuffer>(
        typing_buffer_entity);

    afterhours::Entity &order_queue_entity =
        afterhours::EntityHelper::createEntity();
    OrderQueue &order_queue = order_queue_entity.addComponent<OrderQueue>();
    order_queue.max_active_orders = 3;
    afterhours::EntityHelper::registerSingleton<OrderQueue>(order_queue_entity);

    afterhours::Entity &box_entity = afterhours::EntityHelper::createEntity();
    Box &box = box_entity.addComponent<Box>();
    box.capacity = 10;
    box_entity.enableTag(GameTag::IsBox);
  }

  {
    afterhours::input::register_update_systems(systems);
    afterhours::window_manager::register_update_systems(systems);

    systems.register_update_system(std::make_unique<SpawnItems>());
    systems.register_update_system(std::make_unique<GenerateOrders>());
    systems.register_update_system(std::make_unique<ProcessTypingInput>());
    systems.register_update_system(std::make_unique<MatchItemToOrder>());
    systems.register_update_system(std::make_unique<GrabItem>());
    systems.register_update_system(std::make_unique<BoxItem>());
    systems.register_update_system(std::make_unique<UpdateRenderTexture>());
  }

  {
    systems.register_render_system(std::make_unique<BeginWorldRender>());
    systems.register_render_system(std::make_unique<RenderOrders>());
    systems.register_render_system(std::make_unique<RenderTypingBuffer>());
    systems.register_render_system(std::make_unique<RenderBox>());
    systems.register_render_system(std::make_unique<EndWorldRender>());
    systems.register_render_system(
        std::make_unique<BeginPostProcessingRender>());
    systems.register_render_system(std::make_unique<RenderRenderTexture>());
    afterhours::ui::register_render_systems<InputAction>(
        systems, InputAction::ToggleUILayoutDebug);
    systems.register_render_system(std::make_unique<EndDrawing>());
  }

  while (running && !raylib::WindowShouldClose()) {
    float dt = raylib::GetFrameTime();
    systems.run(dt);
  }
}
