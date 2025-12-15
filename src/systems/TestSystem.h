#pragma once

#include <afterhours/ah.h>

#include <optional>
#include <string>

#include "../testing/test_app.h"

namespace test_app {
extern int frame_counter;
}

struct TestSystem : afterhours::System<> {
    std::optional<TestApp> current_test;
    bool test_complete = false;
    std::string test_error;
    std::string test_name;
    int max_total_frames = 6000;  // hard timeout per test (fail loudly)

    bool is_complete() const { return test_complete; }
    std::string get_error() const { return test_error; }
    std::string get_test_name() const { return test_name; }

    void set_test(const std::string &name, TestApp test) {
        test_name = name;
        current_test = std::move(test);
        test_complete = false;
        test_error.clear();
        test_input::test_mode = true;
        test_app::frame_counter = 0;
    }

    void once(float) override {
        if (!current_test.has_value()) {
            return;
        }

        if (test_complete) {
            return;
        }

        // Hard-stop timeout: if a test doesn't reach the end, it's a failure.
        if (test_app::frame_counter >= max_total_frames) {
            test_error = "Test timed out (did not reach completion) after " +
                         std::to_string(max_total_frames) + " frames\n";
            test_complete = true;
            current_test.reset();
            return;
        }

        if (!current_test->handle) {
            return;
        }

        if (!current_test->handle.done()) {
            auto &promise = current_test->handle.promise();

            // If the coroutine is waiting, only resume when the wait is ready
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

                if (!should_resume) {
                    return;  // Not ready to resume from wait
                }

                promise.wait.kind = TestApp::promise_type::WaitKind::None;
                if (timed_out) {
                    promise.wait_timed_out = true;
                }
            }

            // Resume the coroutine (either from wait or because not waiting)
            current_test->resume();
        }

        if (!current_test->is_done()) {
            return;
        }

        // Handle test completion
        if (current_test->is_successfully_completed()) {
            test_complete = true;
            current_test.reset();
            return;
        }

        std::string error = current_test->get_error();
        if (!error.empty()) {
            test_error = error;
            test_complete = true;
            current_test.reset();
            return;
        }

        if (current_test->get_return_value() != 0) {
            test_error = "Test completed but did not return 0 (returned " +
                         std::to_string(current_test->get_return_value()) + ")";
            test_complete = true;
            current_test.reset();
            return;
        }

        test_error = "Test completed in unknown state";
        test_complete = true;
        current_test.reset();
    }
};