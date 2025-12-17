#include "game.h"

#include <afterhours/src/plugins/animation.h>
#include <afterhours/src/plugins/files.h>

#include <chrono>
#include <thread>

#include "components.h"
#include "input_mapping.h"
#include "log.h"
#include "preload.h"
#include "render_backend.h"
#include "render_views.h"
#include "settings.h"
#include "systems/BoxItemSystem.h"
#include "systems/GenerateOrdersSystem.h"
#include "systems/GrabItemSystem.h"
#include "systems/ManageConveyorItemsSystem.h"
#include "systems/ManageInProgressOrderTagSystem.h"
#include "systems/ManageOrderStateTagsSystem.h"
#include "systems/ManageSelectedOrderTagSystem.h"
#include "systems/MatchItemToOrderSystem.h"
#include "systems/ProcessBoxingInputSystem.h"
#include "systems/ProcessOrderSelectionSystem.h"
#include "systems/ProcessOrderTabbingSystem.h"
#include "systems/ProcessReadyStampSystem.h"
#include "systems/ProcessTypingInputSystem.h"
#include "systems/RenderDebugOverlaySystem.h"
#include "systems/RenderOrders.h"
#include "systems/RenderRenderTextureSystem.h"
#include "systems/RenderSystemHelpers.h"
#include "systems/RenderTypingBufferSystem.h"
#include "systems/SpawnConveyorItemsSystem.h"
#include "systems/SpawnItemsSystem.h"
#include "systems/TestFrameBeginSystem.h"
#include "systems/TestSystem.h"
#include "systems/UpdateOrderWorkflowSystem.h"
#include "systems/UpdateRenderTextureSystem.h"
#include "systems/UpdateShippingAnimationSystem.h"
#include "testing/test_input.h"
#include "testing/test_macros.h"
#include "testing/tests/all_tests.h"

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
        // Create singleton entities for TypingBuffer, OrderQueue, ActiveView,
        // SelectedOrder, and ActiveOrder
        afterhours::Entity &typing_buffer_entity =
            afterhours::EntityHelper::createEntity();
        typing_buffer_entity.addComponent<TypingBuffer>();
        afterhours::EntityHelper::registerSingleton<TypingBuffer>(
            typing_buffer_entity);

        // OrderQueue singleton no longer needed - using OrderSlot components
        // instead

        afterhours::Entity &active_view_entity =
            afterhours::EntityHelper::createEntity();
        active_view_entity.addComponent<ActiveView>();
        afterhours::EntityHelper::registerSingleton<ActiveView>(
            active_view_entity);

        afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::createEntity();
        selected_order_entity.addComponent<SelectedOrder>();
        afterhours::EntityHelper::registerSingleton<SelectedOrder>(
            selected_order_entity);

        afterhours::Entity &active_order_entity =
            afterhours::EntityHelper::createEntity();
        active_order_entity.addComponent<ActiveOrder>();
        afterhours::EntityHelper::registerSingleton<ActiveOrder>(
            active_order_entity);

        afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::createEntity();
        boxing_progress_entity.addComponent<BoxingProgress>();
        afterhours::EntityHelper::registerSingleton<BoxingProgress>(
            boxing_progress_entity);

        afterhours::Entity &shipping_animation_entity =
            afterhours::EntityHelper::createEntity();
        shipping_animation_entity.addComponent<ShippingAnimation>();
        afterhours::EntityHelper::registerSingleton<ShippingAnimation>(
            shipping_animation_entity);

        afterhours::Entity &debug_overlay_entity =
            afterhours::EntityHelper::createEntity();
        debug_overlay_entity.addComponent<DebugOverlay>();
        afterhours::EntityHelper::registerSingleton<DebugOverlay>(
            debug_overlay_entity);

        afterhours::Entity &box_entity =
            afterhours::EntityHelper::createEntity();
        Box &box = box_entity.addComponent<Box>();
        box.capacity = 10;
        box_entity.enableTag(GameTag::IsBox);
    }

    TestSystem *test_system_ptr = nullptr;

    {
        afterhours::input::register_update_systems(systems);
        afterhours::window_manager::register_update_systems(systems);

        systems.register_update_system(
            std::make_unique<TestFrameBeginSystem>());
        systems.register_update_system(std::make_unique<SpawnItemsSystem>());
        systems.register_update_system(
            std::make_unique<ManageInProgressOrderTagSystem>());
        systems.register_update_system(
            std::make_unique<ManageSelectedOrderTagSystem>());
        systems.register_update_system(
            std::make_unique<GenerateOrdersSystem>());
        systems.register_update_system(
            std::make_unique<ProcessOrderSelectionSystem>());
        systems.register_update_system(
            std::make_unique<ProcessOrderTabbingSystem>());
        systems.register_update_system(
            std::make_unique<ProcessReadyStampSystem>());
        systems.register_update_system(
            std::make_unique<SpawnConveyorItemsSystem>());
        systems.register_update_system(
            std::make_unique<ProcessTypingInputSystem>());
        systems.register_update_system(
            std::make_unique<ProcessBoxingInputSystem>());
        systems.register_update_system(
            std::make_unique<UpdateShippingAnimationSystem>());
        systems.register_update_system(
            std::make_unique<MatchItemToOrderSystem>());
        systems.register_update_system(
            std::make_unique<ManageConveyorItemsSystem>());
        systems.register_update_system(std::make_unique<GrabItemSystem>());
        systems.register_update_system(std::make_unique<BoxItemSystem>());
        // New order workflow systems
        systems.register_update_system(
            std::make_unique<ManageOrderStateTagsSystem>());
        systems.register_update_system(
            std::make_unique<UpdateOrderWorkflowSystem>());
        systems.register_update_system(
            std::make_unique<UpdateRenderTextureSystem>());

        auto test_system = std::make_unique<TestSystem>();
        test_system_ptr = test_system.get();
        systems.register_update_system(std::move(test_system));
    }

    {
        systems.register_render_system(
            std::make_unique<BeginWorldRenderSystem>());
        register_render_cutscene_systems(systems);
        register_render_computer_systems(systems);
        register_render_warehouse_systems(systems);
        register_render_boxing_systems(systems);
        systems.register_render_system(
            std::make_unique<RenderTypingBufferSystem>());
        systems.register_render_system(
            std::make_unique<RenderDebugOverlaySystem>());
        systems.register_render_system(
            std::make_unique<EndWorldRenderSystem>());
        systems.register_render_system(
            std::make_unique<BeginPostProcessingRenderSystem>());
        systems.register_render_system(
            std::make_unique<RenderRenderTextureSystem>());
        afterhours::ui::register_render_systems<InputAction>(
            systems, InputAction::ToggleUILayoutDebug);
        systems.register_render_system(std::make_unique<EndDrawingSystem>());
    }

    while (running && !raylib::WindowShouldClose()) {
        if (raylib::IsKeyPressed(raylib::KEY_ESCAPE)) {
            running = false;
        }
        if (raylib::IsKeyPressed(raylib::KEY_F1)) {
            DebugOverlay &debug_overlay = get_singleton_as<DebugOverlay>();
            debug_overlay.enabled = !debug_overlay.enabled;
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
                std::cout << "Test '" << test_system_ptr->get_test_name()
                          << "' passed!" << std::endl;
                running = false;
            }
        }
    }
}

int run_test(const std::string &test_name, bool slow_mode) {
    log_info("run_test: Starting test '{}'", test_name);
    TestRegistry &registry = TestRegistry::get();
    auto it = registry.tests.find(test_name);
    if (it == registry.tests.end()) {
        std::cout << "Test '" << test_name << "' failed: not found"
                  << std::endl;
        return 2;
    }

    test_input::slow_test_mode = slow_mode;
    running = true;

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

        afterhours::Entity &active_view_entity =
            afterhours::EntityHelper::createEntity();
        active_view_entity.addComponent<ActiveView>();
        afterhours::EntityHelper::registerSingleton<ActiveView>(
            active_view_entity);

        afterhours::Entity &selected_order_entity =
            afterhours::EntityHelper::createEntity();
        selected_order_entity.addComponent<SelectedOrder>();
        afterhours::EntityHelper::registerSingleton<SelectedOrder>(
            selected_order_entity);

        afterhours::Entity &active_order_entity =
            afterhours::EntityHelper::createEntity();
        active_order_entity.addComponent<ActiveOrder>();
        afterhours::EntityHelper::registerSingleton<ActiveOrder>(
            active_order_entity);

        afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::createEntity();
        boxing_progress_entity.addComponent<BoxingProgress>();
        afterhours::EntityHelper::registerSingleton<BoxingProgress>(
            boxing_progress_entity);

        afterhours::Entity &shipping_animation_entity =
            afterhours::EntityHelper::createEntity();
        shipping_animation_entity.addComponent<ShippingAnimation>();
        afterhours::EntityHelper::registerSingleton<ShippingAnimation>(
            shipping_animation_entity);

        afterhours::Entity &debug_overlay_entity =
            afterhours::EntityHelper::createEntity();
        debug_overlay_entity.addComponent<DebugOverlay>();
        afterhours::EntityHelper::registerSingleton<DebugOverlay>(
            debug_overlay_entity);

        afterhours::Entity &box_entity =
            afterhours::EntityHelper::createEntity();
        Box &box = box_entity.addComponent<Box>();
        box.capacity = 10;
        box_entity.enableTag(GameTag::IsBox);
    }

    TestSystem *test_system_ptr = nullptr;

    {
        afterhours::input::register_update_systems(systems);
        afterhours::window_manager::register_update_systems(systems);

        systems.register_update_system(
            std::make_unique<TestFrameBeginSystem>());
        systems.register_update_system(std::make_unique<SpawnItemsSystem>());
        systems.register_update_system(
            std::make_unique<ManageInProgressOrderTagSystem>());
        systems.register_update_system(
            std::make_unique<ManageSelectedOrderTagSystem>());
        systems.register_update_system(
            std::make_unique<GenerateOrdersSystem>());
        systems.register_update_system(
            std::make_unique<ProcessOrderSelectionSystem>());
        systems.register_update_system(
            std::make_unique<ProcessOrderTabbingSystem>());
        systems.register_update_system(
            std::make_unique<ProcessReadyStampSystem>());
        systems.register_update_system(
            std::make_unique<SpawnConveyorItemsSystem>());
        systems.register_update_system(
            std::make_unique<ProcessTypingInputSystem>());
        systems.register_update_system(
            std::make_unique<ProcessBoxingInputSystem>());
        systems.register_update_system(
            std::make_unique<UpdateOrderWorkflowSystem>());
        systems.register_update_system(
            std::make_unique<UpdateShippingAnimationSystem>());
        systems.register_update_system(
            std::make_unique<MatchItemToOrderSystem>());
        systems.register_update_system(
            std::make_unique<ManageConveyorItemsSystem>());
        systems.register_update_system(std::make_unique<GrabItemSystem>());
        systems.register_update_system(std::make_unique<BoxItemSystem>());
        systems.register_update_system(
            std::make_unique<UpdateRenderTextureSystem>());

        log_info("Test systems registered, running test...");

        auto test_system = std::make_unique<TestSystem>();
        test_system_ptr = test_system.get();
        systems.register_update_system(std::move(test_system));
    }

    {
        systems.register_render_system(
            std::make_unique<BeginWorldRenderSystem>());
        register_render_cutscene_systems(systems);
        register_render_computer_systems(systems);
        register_render_warehouse_systems(systems);
        register_render_boxing_systems(systems);
        systems.register_render_system(
            std::make_unique<RenderTypingBufferSystem>());
        systems.register_render_system(
            std::make_unique<RenderDebugOverlaySystem>());
        systems.register_render_system(
            std::make_unique<EndWorldRenderSystem>());
        systems.register_render_system(
            std::make_unique<BeginPostProcessingRenderSystem>());
        systems.register_render_system(
            std::make_unique<RenderRenderTextureSystem>());
        afterhours::ui::register_render_systems<InputAction>(
            systems, InputAction::ToggleUILayoutDebug);
        systems.register_render_system(std::make_unique<EndDrawingSystem>());
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
                return 1;
            } else {
                std::cout << "Test '" << test_system_ptr->get_test_name()
                          << "' passed!" << std::endl;
                running = false;
                return 0;
            }
        }
    }

    // If we exited the loop without a result, that's a failure.
    std::cout << "Test '" << test_name
              << "' failed: runner exited without completion" << std::endl;
    return 1;
}
