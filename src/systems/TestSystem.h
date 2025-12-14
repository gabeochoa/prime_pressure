#pragma once

#include <afterhours/ah.h>

#include <chrono>
#include <optional>
#include <thread>

#include "../testing/test_app.h"
#include "../testing/test_input.h"

namespace test_app {
extern int frame_counter;
}

struct TestSystem : afterhours::System<> {
    std::optional<TestApp> current_test;
    std::string test_name;
    bool test_complete = false;
    std::string test_error;
    int max_total_frames = 6000; // hard timeout per test (fail loudly)

    std::optional<OrderState> last_traced_state;

    void reset_trace() {
        test_app::reset_state_trace();
        last_traced_state.reset();
    }

    void trace_selected_order_state() {
        // SelectedOrder is always present as a singleton in run_test()
        afterhours::Entity &sel_entity =
            afterhours::EntityHelper::get_singleton<SelectedOrder>();
        const SelectedOrder &sel = sel_entity.get<SelectedOrder>();
        if (!sel.order_id.order_id.has_value()) {
            return;
        }
        auto order_id = sel.order_id.order_id.value();
        auto order_opt =
            afterhours::EntityQuery()
                .whereID(order_id)
                .whereHasComponent<OrderWorkflow>()
                .gen_first();
        if (!order_opt.has_value()) {
            return;
        }
        const afterhours::Entity &order_entity = order_opt.asE();
        const OrderWorkflow &workflow = order_entity.get<OrderWorkflow>();
        OrderState s = workflow.state;
        if (!last_traced_state.has_value() || last_traced_state.value() != s) {
            test_app::record_state_trace(test_app::frame_counter, s);
            last_traced_state = s;
        }
    }

    std::string format_trace() const {
        if (test_app::state_trace.empty()) {
            return "State trace: <empty>";
        }
        std::string out = "State trace (frame: state):\n";
        for (const auto &[frame, st] : test_app::state_trace) {
            out += "  " + std::to_string(frame) + ": " +
                   std::string(magic_enum::enum_name(st)) + "\n";
        }
        return out;
    }

    void set_test(const std::string &name, TestApp test) {
        log_info("TestSystem: Setting test '{}'", name);
        test_name = name;
        current_test = std::move(test);
        test_complete = false;
        test_error.clear();
        test_input::test_mode = true;
        test_input::clear_queue();
        test_app::frame_counter = 0;
        reset_trace();
    }

    void once(float) override {
        // NOTE: frame counter + input reset happens in TestFrameBeginSystem
        // (runs early in the frame).
        if (current_test.has_value()) {
            // Log every 10 frames to avoid spam
            if (test_app::frame_counter % 10 == 0) {
                log_info("TestSystem: Running frame {}",
                         test_app::frame_counter);
            }
        }

        if (!current_test.has_value()) {
            return;
        }

        if (test_complete) {
            return;
        }

        // Trace the selected order's state (if any) each frame.
        trace_selected_order_state();

        // Hard-stop timeout: if a test doesn't reach the end, it's a failure.
        if (test_app::frame_counter >= max_total_frames) {
            test_error =
                "Test timed out (did not reach completion) after " +
                std::to_string(max_total_frames) + " frames\n" + format_trace();
            test_complete = true;
            test_input::test_mode = false;
            current_test.reset();
            return;
        }

        if (current_test->handle && !current_test->handle.done()) {
            auto &promise = current_test->handle.promise();

            // If the coroutine is waiting, only resume when the wait is ready
            // (or timed out).
            if (promise.wait.kind != TestApp::promise_type::WaitKind::None) {
                bool should_resume = false;
                bool timed_out = false;

                if (promise.wait.kind ==
                    TestApp::promise_type::WaitKind::Frames) {
                    should_resume =
                        test_app::frame_counter >= promise.wait.target_frame;
                } else if (promise.wait.kind ==
                           TestApp::promise_type::WaitKind::Condition) {
                    bool cond_met =
                        promise.wait.condition && promise.wait.condition();
                    bool cond_timeout =
                        (test_app::frame_counter - promise.wait.start_frame) >=
                        promise.wait.max_frames;
                    should_resume = cond_met || cond_timeout;
                    timed_out = (!cond_met && cond_timeout);
                }

                if (should_resume) {
                    // Clear wait before resuming to avoid re-triggering.
                    promise.wait.kind = TestApp::promise_type::WaitKind::None;
                    if (timed_out) {
                        promise.wait_timed_out = true;
                    }
                    current_test->resume();
                }
            } else {
                // Not waiting on an awaitable; run one step this frame.
                current_test->resume();
            }

            if (test_input::slow_test_mode) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        }

        if (current_test->is_done()) {
            // Check if test completed successfully with return 0
            if (current_test->is_successfully_completed()) {
                test_complete = true;
                test_input::test_mode = false;
                current_test.reset();
            } else {
                // Test completed but didn't return 0 - keep running (don't mark
                // as complete)
                std::string error = current_test->get_error();
                if (!error.empty()) {
                    test_error = error + "\n" + format_trace();
                    test_complete = true;  // Mark as complete with error
                    test_input::test_mode = false;
                    current_test.reset();
                } else if (current_test->get_return_value() != 0) {
                    // Test returned non-zero but no exception - this is a
                    // failure
                    test_error =
                        "Test completed but did not return 0 (returned " +
                        std::to_string(current_test->get_return_value()) +
                        ")\n" + format_trace();
                    test_complete = true;
                    test_input::test_mode = false;
                    current_test.reset();
                }
                // If return_value is still -1 and no error, keep running
            }
        }
    }

    bool is_complete() const { return test_complete; }
    std::string get_error() const { return test_error; }
    std::string get_test_name() const { return test_name; }
};
