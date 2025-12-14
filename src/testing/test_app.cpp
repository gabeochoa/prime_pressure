#include "test_app.h"

namespace test_app {
int frame_counter = 0;
std::vector<std::pair<int, OrderState>> state_trace;

void reset_state_trace() { state_trace.clear(); }

void record_state_trace(int frame, OrderState state) {
    // only record transitions
    if (!state_trace.empty() && state_trace.back().second == state) {
        return;
    }
    state_trace.push_back({frame, state});
}

bool was_state_seen(OrderState state) {
    for (const auto &p : state_trace) {
        if (p.second == state) return true;
    }
    return false;
}
}
