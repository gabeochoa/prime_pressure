#include "mcp_server.h"
#include "../components.h"
#include "../input_wrapper.h"
#include "../systems/TestSystem.h"
#include "../testing/test_input.h"
#include "../testing/test_macros.h"
#include <afterhours/ah.h>
#include <iostream>

using json = nlohmann::json;

MCPServer::MCPServer() {}

MCPServer::~MCPServer() { stop(); }

void MCPServer::set_test_system(TestSystem *test_system) {
  test_system_ptr = test_system;
}

void MCPServer::start() {
  if (running)
    return;
  running = true;
  worker_thread = std::thread(&MCPServer::run_loop, this);
}

void MCPServer::stop() {
  running = false;
  if (worker_thread.joinable()) {
    worker_thread.join();
  }
}

void MCPServer::run_loop() {
  std::string line;
  while (running && std::getline(std::cin, line)) {
    try {
      auto request = json::parse(line);

      if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0") {
        continue; // Ignore invalid JSON-RPC
      }

      std::string method = request["method"];
      std::string id = request.contains("id") ? request["id"].dump() : "";
      // Remove quotes from id dump if present (simple hack)
      if (id.size() >= 2 && id.front() == '"' && id.back() == '"') {
        id = id.substr(1, id.size() - 2);
      }

      if (method == "initialize") {
        send_response(id, handle_initialize(request["params"]));
      } else if (method == "notifications/initialized") {
        // No response needed
      } else if (method == "tools/list") {
        send_response(id, handle_list_tools(request["params"]));
      } else if (method == "tools/call") {
        // Push to command queue for main thread processing
        CommandQueue::Command cmd;
        cmd.method = method;
        cmd.params = request["params"];
        cmd.id = id;
        command_queue.push(cmd);
      } else {
        // Unknown method
        // For now, just ignore or log
      }
    } catch (const std::exception &e) {
      // Log error to stderr so it doesn't interfere with stdout JSON-RPC
      std::cerr << "MCP JSON Parse Error: " << e.what() << std::endl;
    }
  }
}

void MCPServer::process_commands() {
  while (true) {
    auto cmd_opt = command_queue.pop();
    if (!cmd_opt)
      break;

    auto &cmd = cmd_opt.value();

    if (cmd.method == "tools/call") {
      try {
        std::string name = cmd.params["name"];
        json args = cmd.params["arguments"];
        json result;

        if (name == "get_state") {
          // Gather game state
          const afterhours::Entity &view_entity =
              afterhours::EntityHelper::get_singleton<ActiveView>();
          const ActiveView &active_view = view_entity.get<ActiveView>();

          const afterhours::Entity &buffer_entity =
              afterhours::EntityHelper::get_singleton<TypingBuffer>();
          const TypingBuffer &buffer = buffer_entity.get<TypingBuffer>();

          const afterhours::Entity &queue_entity =
              afterhours::EntityHelper::get_singleton<OrderQueue>();
          const OrderQueue &queue = queue_entity.get<OrderQueue>();

          const afterhours::Entity &selected_order_entity =
              afterhours::EntityHelper::get_singleton<SelectedOrder>();
          const SelectedOrder &selected_order =
              selected_order_entity.get<SelectedOrder>();

          result["view"] =
              (active_view.current_view == ViewState::Computer) ? "computer"
              : (active_view.current_view == ViewState::Boxing) ? "boxing"
                                                                : "warehouse";
          result["typing_buffer"] = buffer.buffer;

          if (selected_order.order_id.has_value()) {
            result["selected_order_id"] = selected_order.order_id.value();
          } else {
            result["selected_order_id"] = nullptr;
          }

          // Orders
          std::vector<json> orders;
          for (auto order_id : queue.in_progress_orders) {
            if (order_id != -1) {
              auto opt_order_ent =
                  afterhours::EntityHelper::getEntityForID(order_id);
              if (!opt_order_ent.has_value())
                continue;

              const Order &order = opt_order_ent.value()->get<Order>();
              json order_json;
              order_json["id"] = order_id;
              order_json["is_selected"] =
                  (selected_order.order_id.has_value() &&
                   selected_order.order_id.value() == order_id);

              std::vector<std::string> items;
              auto selected_copy = order.selected_items;
              for (auto item_type : order.items) {
                std::string item_str = item_type_to_string(item_type);
                // Check if this instance is selected
                auto it = std::find(selected_copy.begin(), selected_copy.end(),
                                    item_type);
                if (it != selected_copy.end()) {
                  item_str += " (selected)";
                  selected_copy.erase(it);
                }
                items.push_back(item_str);
              }
              order_json["items"] = items;
              orders.push_back(order_json);
            }
          }
          result["orders"] = orders;

        } else if (name == "type_text") {
          std::string text = args["text"];
          for (char c : text) {
            game_input::InjectChar(static_cast<int>(c));
          }
          result = "Typed: " + text;
        } else if (name == "press_key") {
          std::string key_name = args["key"];
          int key_code = 0;
          if (key_name == "TAB")
            key_code = 258; // KEY_TAB
          else if (key_name == "ENTER")
            key_code = 257; // KEY_ENTER
          else if (key_name == "1")
            key_code = 49;
          else if (key_name == "2")
            key_code = 50;
          else if (key_name == "3")
            key_code = 51;
          // Add more keys as needed

          if (key_code != 0) {
            game_input::InjectKey(key_code);
            result = "Pressed: " + key_name;
          } else {
            result = "Unknown key: " + key_name;
          }
        } else if (name == "list_tests") {
          TestRegistry &registry = TestRegistry::get();
          std::vector<std::string> test_names;
          for (const auto &[test_name, func] : registry.tests) {
            test_names.push_back(test_name);
          }
          result["tests"] = test_names;
        } else if (name == "run_test") {
          if (!test_system_ptr) {
            send_error(cmd.id, -32603, "Test system not available");
            continue;
          }
          std::string test_name = args["test_name"];
          bool slow_mode = args.value("slow_mode", false);

          TestRegistry &registry = TestRegistry::get();
          auto it = registry.tests.find(test_name);
          if (it == registry.tests.end()) {
            send_error(cmd.id, -32602, "Test not found: " + test_name);
            continue;
          }

          TestApp test = it->second();
          test_system_ptr->set_test(test_name, std::move(test));
          test_input::slow_test_mode = slow_mode;

          result = "Started test: " + test_name;
        } else if (name == "get_test_status") {
          if (!test_system_ptr) {
            result["status"] = "no_test_system";
            result["running"] = false;
          } else if (test_system_ptr->is_complete()) {
            result["status"] = "complete";
            result["running"] = false;
            result["test_name"] = test_system_ptr->get_test_name();
            std::string error = test_system_ptr->get_error();
            if (!error.empty()) {
              result["error"] = error;
              result["passed"] = false;
            } else {
              result["passed"] = true;
            }
          } else {
            result["status"] = "running";
            result["running"] = true;
            result["test_name"] = test_system_ptr->get_test_name();
          }
        } else {
          send_error(cmd.id, -32601, "Tool not found: " + name);
          continue;
        }

        json response;
        response["content"] = {{{"type", "text"}, {"text", result.dump()}}};
        send_response(cmd.id, response);

      } catch (const std::exception &e) {
        send_error(cmd.id, -32603, std::string("Internal error: ") + e.what());
      }
    }
  }
}

json MCPServer::handle_initialize(const json &params) {
  (void)params;
  json result;
  result["protocolVersion"] = "2024-11-05";
  result["capabilities"] = {{"tools", {{"listChanged", false}}}};
  result["serverInfo"] = {{"name", "PrimePressureMCP"}, {"version", "1.0.0"}};
  return result;
}

json MCPServer::handle_list_tools(const json &params) {
  (void)params;
  json result;
  result["tools"] = {
      {{"name", "get_state"},
       {"description", "Get the current game state (view, orders, buffer)"},
       {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}},
      {{"name", "type_text"},
       {"description", "Type text into the game"},
       {"inputSchema",
        {{"type", "object"},
         {"properties",
          {{"text", {{"type", "string"}, {"description", "Text to type"}}}}}}}},
      {{"name", "press_key"},
       {"description", "Press a special key (TAB, ENTER, 1, 2, 3)"},
       {"inputSchema",
        {{"type", "object"},
         {"properties",
          {{"key", {{"type", "string"}, {"description", "Key name"}}}}}}}},
      {{"name", "list_tests"},
       {"description", "List all available tests"},
       {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}},
      {{"name", "run_test"},
       {"description", "Run a specific test"},
       {"inputSchema",
        {{"type", "object"},
         {"properties",
          {{"test_name",
            {{"type", "string"}, {"description", "Name of the test to run"}}},
           {"slow_mode",
            {{"type", "boolean"},
             {"description", "Run test in slow mode (visible)"}}}}}}}},
      {{"name", "get_test_status"},
       {"description", "Get the status of the currently running test"},
       {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}}};
  return result;
}

void MCPServer::send_response(const std::string &id, const json &result) {
  json response;
  response["jsonrpc"] = "2.0";
  response["id"] = id;
  response["result"] = result;
  std::cout << response.dump() << std::endl;
}

void MCPServer::send_error(const std::string &id, int code,
                           const std::string &message) {
  json response;
  response["jsonrpc"] = "2.0";
  response["id"] = id;
  response["error"] = {{"code", code}, {"message", message}};
  std::cout << response.dump() << std::endl;
}
