#pragma once

#include <afterhours/ah.h>

#include "../components.h"
#include "../order_components.h"
#include "../order_state_machine.h"
#include "../rl.h"

struct RenderDebugOverlaySystem : afterhours::System<> {
    void once(float) const override {
        DebugOverlay &debug_overlay = get_singleton_as<DebugOverlay>();
        if (!debug_overlay.enabled) {
            return;
        }

        // Semi-transparent background for debug overlay
        raylib::DrawRectangle(10, 10, 700, 500,
                              raylib::Fade(raylib::BLACK, 0.8f));
        raylib::DrawRectangleLines(10, 10, 700, 500, raylib::YELLOW);

        // Title
        raylib::DrawTextEx(uiFont, "DEBUG OVERLAY (Press F1 to toggle)",
                           {20.0f, 20.0f}, 20.0f, 1.0f, raylib::YELLOW);

        float y = 50.0f;

        // Add order workflow information
        for (const afterhours::Entity &entity :
             afterhours::EntityQuery()
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const OrderWorkflow &workflow = entity.get<OrderWorkflow>();
            std::string workflow_info =
                "Order " +
                std::to_string(static_cast<unsigned long long>(entity.id)) +
                ": macro=" +
                std::to_string(
                    static_cast<int>(macro_state_of(workflow.state))) +
                ", micro=" + std::to_string(static_cast<int>(workflow.state)) +
                ", kind=" +
                std::to_string(static_cast<int>(kind_of(workflow.state))) +
                ", time=" + std::to_string(workflow.time_in_state);

            if (y > 470) {  // Don't draw beyond the overlay bounds
                raylib::DrawTextEx(uiFont, "... (truncated)", {20.0f, y}, 15.0f,
                                   1.0f, raylib::GRAY);
                break;
            }
            raylib::DrawTextEx(uiFont, workflow_info.c_str(), {20.0f, y}, 15.0f,
                               1.0f, raylib::GREEN);
            y += 18.0f;
        }

        // Add a separator
        if (y < 470) {
            raylib::DrawTextEx(uiFont, "--- System Messages ---", {20.0f, y},
                               15.0f, 1.0f, raylib::YELLOW);
            y += 20.0f;
        }

        // Display debug information from systems
        for (const auto &line : debug_overlay.debug_lines) {
            if (y > 470) {  // Don't draw beyond the overlay bounds
                raylib::DrawTextEx(uiFont, "... (truncated)", {20.0f, y}, 15.0f,
                                   1.0f, raylib::GRAY);
                break;
            }
            raylib::DrawTextEx(uiFont, line.c_str(), {20.0f, y}, 15.0f, 1.0f,
                               raylib::WHITE);
            y += 18.0f;
        }

        // Clear debug lines after displaying (they'll be repopulated each
        // frame)
        debug_overlay.debug_lines.clear();
    }
};