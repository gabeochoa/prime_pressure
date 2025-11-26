#pragma once

#include "../components.h"
#include "../rl.h"
#include "test_input.h"
#include <afterhours/ah.h>
#include <coroutine>
#include <string>
#include <vector>
#include <functional>

namespace test_app {
extern int frame_counter;
}

struct TestApp {
  struct promise_type {
    std::string error_message;
    bool done = false;
    
    TestApp get_return_object() {
      return TestApp{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() {
      try {
        std::rethrow_exception(std::current_exception());
      } catch (const std::exception& e) {
        error_message = e.what();
      }
      done = true;
    }
    void return_void() { done = true; }
  };
  
  std::coroutine_handle<promise_type> handle;
  
  TestApp(std::coroutine_handle<promise_type> h) : handle(h) {}
  
  TestApp(const TestApp&) = delete;
  TestApp& operator=(const TestApp&) = delete;
  
  TestApp(TestApp&& other) noexcept : handle(other.handle) {
    other.handle = {};
  }
  
  TestApp& operator=(TestApp&& other) noexcept {
    if (this != &other) {
      if (handle) {
        handle.destroy();
      }
      handle = other.handle;
      other.handle = {};
    }
    return *this;
  }
  
  ~TestApp() { 
    if (handle) {
      handle.destroy();
    }
  }
  
  bool is_done() const { 
    if (!handle) return true;
    return handle.promise().done; 
  }
  
  std::string get_error() const { 
    if (!handle) return "";
    return handle.promise().error_message; 
  }
  
  void resume() {
    if (handle && !handle.done()) {
      handle.resume();
    }
  }
  
  static void simulate_key(int key) {
    test_input::push_key(key);
  }
  
  static void simulate_char(char c) {
    test_input::push_char(c);
  }
  
  static void simulate_typing(const std::string& text) {
    for (char c : text) {
      test_input::push_char(c);
    }
  }
  
  struct WaitFrames {
    int target_frame;
    int start_frame;
    int slow_multiplier;
    
    WaitFrames(int frames) : target_frame(test_app::frame_counter + frames), start_frame(test_app::frame_counter), slow_multiplier(test_input::slow_test_mode ? 100 : 1) {
      if (test_input::slow_test_mode) {
        target_frame = test_app::frame_counter + (frames * slow_multiplier);
      }
    }
    
    bool await_ready() const {
      return test_app::frame_counter >= target_frame;
    }
    void await_suspend(std::coroutine_handle<promise_type>) {}
    void await_resume() {}
  };
  
  static WaitFrames wait_for_frames(int frames) {
    return WaitFrames{frames};
  }
  
  template<typename Func>
  struct WaitCondition {
    Func condition;
    int max_frames;
    int start_frame;
    
    WaitCondition(Func cond, int max) : condition(cond), max_frames(max), start_frame(test_app::frame_counter) {}
    
    bool await_ready() const {
      if (condition()) {
        return true;
      }
      if (test_app::frame_counter - start_frame >= max_frames) {
        return true;
      }
      return false;
    }
    void await_suspend(std::coroutine_handle<promise_type>) {}
    bool await_resume() {
      bool result = condition();
      if (!result && (test_app::frame_counter - start_frame >= max_frames)) {
        throw std::runtime_error("Condition not met within max frames");
      }
      return result;
    }
  };
  
  template<typename Func>
  static WaitCondition<Func> wait_for_condition(Func condition, int max_frames = 300) {
    return WaitCondition<Func>{condition, max_frames};
  }
  
  static bool has_order() {
    try {
      const afterhours::Entity &queue_entity =
          afterhours::EntityHelper::get_singleton<OrderQueue>();
      const OrderQueue &queue = queue_entity.get<OrderQueue>();
      return !queue.active_orders.empty();
    } catch (...) {
      return false;
    }
  }
  
  static bool is_order_selected() {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();
    return selected_order.order_id.has_value();
  }
  
  static bool are_all_items_selected() {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();
    
    if (!selected_order.order_id.has_value()) {
      return false;
    }
    
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();
      return all_items_selected(order);
    }
    return false;
  }
  
  static bool is_order_shipped() {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();
    
    if (!selected_order.order_id.has_value()) {
      return false;
    }
    
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();
      return order.is_shipped;
    }
    return false;
  }
  
  static bool is_order_fully_complete() {
    const afterhours::Entity &queue_entity =
        afterhours::EntityHelper::get_singleton<OrderQueue>();
    const OrderQueue &queue = queue_entity.get<OrderQueue>();
    
    for (afterhours::EntityID order_id : queue.active_orders) {
      for (const afterhours::Entity &order_entity :
           afterhours::EntityQuery()
               .whereID(order_id)
               .whereHasComponent<Order>()
               .gen()) {
        const Order &order = order_entity.get<Order>();
        if (order.is_fully_complete) {
          return true;
        }
      }
    }
    return false;
  }
  
  static ViewState get_current_view() {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view;
  }
  
  static std::vector<ItemType> get_order_items() {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();
    
    if (!selected_order.order_id.has_value()) {
      return {};
    }
    
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();
      return order.items;
    }
    return {};
  }
  
  static BoxingState get_boxing_state() {
    const afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    const BoxingProgress &boxing_progress = boxing_progress_entity.get<BoxingProgress>();
    return boxing_progress.state;
  }
  
  static int get_items_placed() {
    const afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    const BoxingProgress &boxing_progress = boxing_progress_entity.get<BoxingProgress>();
    return boxing_progress.items_placed;
  }
  
  static int get_total_items_to_box() {
    const afterhours::Entity &selected_order_entity =
        afterhours::EntityHelper::get_singleton<SelectedOrder>();
    const SelectedOrder &selected_order = selected_order_entity.get<SelectedOrder>();
    
    if (!selected_order.order_id.has_value()) {
      return 0;
    }
    
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(selected_order.order_id.value())
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();
      return static_cast<int>(order.selected_items.size());
    }
    return 0;
  }
};

