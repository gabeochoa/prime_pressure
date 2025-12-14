#pragma once

#include <afterhours/ah.h>

#include <stdexcept>

#include "../../components.h"
#include "../../order_components.h"
#include "../../rl.h"
#include "../test_app.h"
#include "../test_macros.h"

TEST(test_complete_order) {
    log_info("TestCompleteOrder: Starting test - UNIQUE MARKER ABC123");
    // Wait for systems to initialize
    co_await TestApp::wait_for_frames(10);
    log_info("TestCompleteOrder: Systems initialized - UNIQUE MARKER DEF456");

    // Check final state
    auto final_state = TestApp::get_selected_order_state();
    log_info("TestCompleteOrder: Final state: {}",
             magic_enum::enum_name(final_state));

    log_info("TestCompleteOrder: Test completed with marker XYZ999");
    co_return 0;
}
