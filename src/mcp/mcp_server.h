#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

struct TestSystem;

// Simple thread-safe queue for passing commands from MCP thread to Main thread
struct CommandQueue {
  struct Command {
    std::string method;
    nlohmann::json params;
    std::string id; // JSON-RPC id
  };

  void push(const Command &cmd) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(cmd);
  }

  std::optional<Command> pop() {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
      return std::nullopt;
    }
    Command cmd = queue.front();
    queue.pop();
    return cmd;
  }

private:
  std::queue<Command> queue;
  std::mutex mutex;
};

class MCPServer {
public:
  MCPServer();
  ~MCPServer();

  // Start the background thread
  void start();

  // Stop the background thread
  void stop();

  // Process pending commands in the main thread
  void process_commands();

  // Set the test system pointer for test-related tools
  void set_test_system(TestSystem *test_system);

private:
  // The loop that runs in the background thread
  void run_loop();

  // Handlers for specific MCP tools/methods
  nlohmann::json handle_initialize(const nlohmann::json &params);
  nlohmann::json handle_list_tools(const nlohmann::json &params);
  nlohmann::json handle_call_tool(const nlohmann::json &params);

  // Helper to send JSON-RPC response
  void send_response(const std::string &id, const nlohmann::json &result);
  void send_error(const std::string &id, int code, const std::string &message);

  std::atomic<bool> running{false};
  std::thread worker_thread;
  CommandQueue command_queue;
  TestSystem *test_system_ptr{nullptr};
};
