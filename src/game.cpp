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
#include "systems/ManageConveyorItems.h"
#include "systems/ManageInProgressOrderTag.h"
#include "systems/ManageSelectedOrderTag.h"
#include "systems/MatchItemToOrder.h"
#include "systems/ProcessBoxingInput.h"
#include "systems/ProcessOrderSelection.h"
#include "systems/ProcessTypingInput.h"
#include "systems/ProcessViewSwitch.h"
#include "systems/RenderBox.h"
#include "systems/RenderBoxingView.h"
#include "systems/RenderComputerView.h"
#include "systems/RenderOrders.h"
#include "systems/RenderRenderTexture.h"
#include "systems/RenderSystemHelpers.h"
#include "systems/RenderTypingBuffer.h"
#include "systems/RenderWarehouseView.h"
#include "systems/SpawnConveyorItems.h"
#include "systems/SpawnItems.h"
#include "systems/TestSystem.h"
#include "systems/UpdateRenderTexture.h"
#include "testing/test_input.h"
#include "testing/test_macros.h"
#include "testing/tests/all_tests.h"
#include <afterhours/src/plugins/files.h>

#include <afterhours/src/plugins/animation.h>
#include <chrono>
#include <thread>

bool running = true;
raylib::RenderTexture2D mainRT;
raylib::RenderTexture2D screenRT;
raylib::Font uiFont;

void game() {
  mainRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                     Settings::get().get_screen_height());
  screenRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                       Settings::get().get_screen_height());
  uiFont = raylib::LoadFont(
      afterhours::files::get_resource_path("fonts", "Gaegu-Bold.ttf")
          .string()
          .c_str());

  afterhours::SystemManager systems;

  {
    afterhours::window_manager::enforce_singletons(systems);
    afterhours::ui::enforce_singletons<InputAction>(systems);
    afterhours::input::enforce_singletons(systems);
  }

  {
    // Create singleton entities for TypingBuffer, OrderQueue, ActiveView, and
    // SelectedOrder
    afterhours::Entity &typing_buffer_entity =
        afterhours::EntityHelper::createEntity();
    typing_buffer_entity.addComponent<TypingBuffer>();
    afterhours::EntityHelper::registerSingleton<TypingBuffer>(
        typing_buffer_entity);

    afterhours::Entity &order_queue_entity =
        afterhours::EntityHelper::createEntity();
    OrderQueue &order_queue = order_queue_entity.addComponent<OrderQueue>();
    order_queue.max_in_progress_orders = 3;
    afterhours::EntityHelper::registerSingleton<OrderQueue>(order_queue_entity);

    afterhours::Entity &active_view_entity =
        afterhours::EntityHelper::createEntity();
    active_view_entity.addComponent<ActiveView>();
    afterhours::EntityHelper::registerSingleton<ActiveView>(active_view_entity);

    afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::createEntity();
    selected_order_entity.addComponent<SelectedOrder>();
    afterhours::EntityHelper::registerSingleton<SelectedOrder>(
        selected_order_entity);

    afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::createEntity();
    boxing_progress_entity.addComponent<BoxingProgress>();
    afterhours::EntityHelper::registerSingleton<BoxingProgress>(
        boxing_progress_entity);

    afterhours::Entity &box_entity = afterhours::EntityHelper::createEntity();
    Box &box = box_entity.addComponent<Box>();
    box.capacity = 10;
    box_entity.enableTag(GameTag::IsBox);
  }

  TestSystem *test_system_ptr = nullptr;

  {
    afterhours::input::register_update_systems(systems);
    afterhours::window_manager::register_update_systems(systems);

    systems.register_update_system(std::make_unique<SpawnItems>());
    systems.register_update_system(
        std::make_unique<ManageInProgressOrderTag>());
    systems.register_update_system(std::make_unique<ManageSelectedOrderTag>());
    systems.register_update_system(std::make_unique<GenerateOrders>());
    systems.register_update_system(std::make_unique<ProcessViewSwitch>());
    systems.register_update_system(std::make_unique<ProcessOrderSelection>());
    systems.register_update_system(std::make_unique<SpawnConveyorItems>());
    systems.register_update_system(std::make_unique<ProcessTypingInput>());
    systems.register_update_system(std::make_unique<ProcessBoxingInput>());
    systems.register_update_system(std::make_unique<MatchItemToOrder>());
    systems.register_update_system(std::make_unique<ManageConveyorItems>());
    systems.register_update_system(std::make_unique<GrabItem>());
    systems.register_update_system(std::make_unique<BoxItem>());
    systems.register_update_system(std::make_unique<UpdateRenderTexture>());

    auto test_system = std::make_unique<TestSystem>();
    test_system_ptr = test_system.get();
    systems.register_update_system(std::move(test_system));
  }

  {
    systems.register_render_system(std::make_unique<BeginWorldRender>());
    systems.register_render_system(std::make_unique<RenderComputerView>());
    systems.register_render_system(std::make_unique<RenderWarehouseView>());
    systems.register_render_system(std::make_unique<RenderBoxingView>());
    systems.register_render_system(std::make_unique<RenderTypingBuffer>());
    systems.register_render_system(std::make_unique<EndWorldRender>());
    systems.register_render_system(
        std::make_unique<BeginPostProcessingRender>());
    systems.register_render_system(std::make_unique<RenderRenderTexture>());
    afterhours::ui::register_render_systems<InputAction>(
        systems, InputAction::ToggleUILayoutDebug);
    systems.register_render_system(std::make_unique<EndDrawing>());
  }

  while (running && !raylib::WindowShouldClose()) {
    if (raylib::IsKeyPressed(raylib::KEY_ESCAPE)) {
      running = false;
    }
    float dt = raylib::GetFrameTime();
    systems.run(dt);

    if (test_system_ptr && test_system_ptr->is_complete()) {
      std::string error = test_system_ptr->get_error();
      if (!error.empty()) {
        std::cout << "Test '" << test_system_ptr->get_test_name()
                  << "' failed: " << error << std::endl;
        running = false;
      } else {
        std::cout << "Test '" << test_system_ptr->get_test_name() << "' passed!"
                  << std::endl;
        running = false;
      }
    }
  }
}

void run_test(const std::string &test_name, bool slow_mode) {
  TestRegistry &registry = TestRegistry::get();
  auto it = registry.tests.find(test_name);
  if (it == registry.tests.end()) {
    std::cout << "Test '" << test_name << "' not found" << std::endl;
    return;
  }

  test_input::slow_test_mode = slow_mode;

  mainRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                     Settings::get().get_screen_height());
  screenRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                       Settings::get().get_screen_height());
  uiFont = raylib::LoadFont(
      afterhours::files::get_resource_path("fonts", "Gaegu-Bold.ttf")
          .string()
          .c_str());

  afterhours::SystemManager systems;

  {
    afterhours::window_manager::enforce_singletons(systems);
    afterhours::ui::enforce_singletons<InputAction>(systems);
    afterhours::input::enforce_singletons(systems);
  }

  {
    afterhours::Entity &typing_buffer_entity =
        afterhours::EntityHelper::createEntity();
    typing_buffer_entity.addComponent<TypingBuffer>();
    afterhours::EntityHelper::registerSingleton<TypingBuffer>(
        typing_buffer_entity);

    afterhours::Entity &order_queue_entity =
        afterhours::EntityHelper::createEntity();
    OrderQueue &order_queue = order_queue_entity.addComponent<OrderQueue>();
    order_queue.max_in_progress_orders = 3;
    afterhours::EntityHelper::registerSingleton<OrderQueue>(order_queue_entity);

    afterhours::Entity &active_view_entity =
        afterhours::EntityHelper::createEntity();
    active_view_entity.addComponent<ActiveView>();
    afterhours::EntityHelper::registerSingleton<ActiveView>(active_view_entity);

    afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::createEntity();
    selected_order_entity.addComponent<SelectedOrder>();
    afterhours::EntityHelper::registerSingleton<SelectedOrder>(
        selected_order_entity);

    afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::createEntity();
    boxing_progress_entity.addComponent<BoxingProgress>();
    afterhours::EntityHelper::registerSingleton<BoxingProgress>(
        boxing_progress_entity);

    afterhours::Entity &box_entity = afterhours::EntityHelper::createEntity();
    Box &box = box_entity.addComponent<Box>();
    box.capacity = 10;
    box_entity.enableTag(GameTag::IsBox);
  }

  TestSystem *test_system_ptr = nullptr;

  {
    afterhours::input::register_update_systems(systems);
    afterhours::window_manager::register_update_systems(systems);

    systems.register_update_system(std::make_unique<SpawnItems>());
    systems.register_update_system(
        std::make_unique<ManageInProgressOrderTag>());
    systems.register_update_system(std::make_unique<ManageSelectedOrderTag>());
    systems.register_update_system(std::make_unique<GenerateOrders>());
    systems.register_update_system(std::make_unique<ProcessViewSwitch>());
    systems.register_update_system(std::make_unique<ProcessOrderSelection>());
    systems.register_update_system(std::make_unique<SpawnConveyorItems>());
    systems.register_update_system(std::make_unique<ProcessTypingInput>());
    systems.register_update_system(std::make_unique<ProcessBoxingInput>());
    systems.register_update_system(std::make_unique<MatchItemToOrder>());
    systems.register_update_system(std::make_unique<ManageConveyorItems>());
    systems.register_update_system(std::make_unique<GrabItem>());
    systems.register_update_system(std::make_unique<BoxItem>());
    systems.register_update_system(std::make_unique<UpdateRenderTexture>());

    auto test_system = std::make_unique<TestSystem>();
    test_system_ptr = test_system.get();
    systems.register_update_system(std::move(test_system));
  }

  {
    systems.register_render_system(std::make_unique<BeginWorldRender>());
    systems.register_render_system(std::make_unique<RenderComputerView>());
    systems.register_render_system(std::make_unique<RenderWarehouseView>());
    systems.register_render_system(std::make_unique<RenderBoxingView>());
    systems.register_render_system(std::make_unique<RenderTypingBuffer>());
    systems.register_render_system(std::make_unique<EndWorldRender>());
    systems.register_render_system(
        std::make_unique<BeginPostProcessingRender>());
    systems.register_render_system(std::make_unique<RenderRenderTexture>());
    afterhours::ui::register_render_systems<InputAction>(
        systems, InputAction::ToggleUILayoutDebug);
    systems.register_render_system(std::make_unique<EndDrawing>());
  }

  TestApp test = it->second();
  test_system_ptr->set_test(test_name, std::move(test));

  while (running && !raylib::WindowShouldClose()) {
    if (raylib::IsKeyPressed(raylib::KEY_ESCAPE)) {
      running = false;
    }
    float dt = raylib::GetFrameTime();
    systems.run(dt);

    if (test_input::slow_test_mode) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (test_system_ptr && test_system_ptr->is_complete()) {
      std::string error = test_system_ptr->get_error();
      if (!error.empty()) {
        std::cout << "Test '" << test_system_ptr->get_test_name()
                  << "' failed: " << error << std::endl;
        running = false;
      } else {
        std::cout << "Test '" << test_system_ptr->get_test_name() << "' passed!"
                  << std::endl;
        running = false;
      }
    }
  }
}
