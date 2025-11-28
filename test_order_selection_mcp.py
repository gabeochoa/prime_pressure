import subprocess
import json
import time
import sys

def send_request(process, method, params=None, req_id=1):
    request = {
        "jsonrpc": "2.0",
        "method": method,
        "id": req_id
    }
    if params is not None:
        request["params"] = params
    
    json_str = json.dumps(request)
    process.stdin.write(json_str + "\n")
    process.stdin.flush()

def read_response(process):
    line = process.stdout.readline()
    if not line:
        return None
    return json.loads(line)

def call_tool(process, tool_name, arguments, req_id=1):
    send_request(process, "tools/call", {
        "name": tool_name,
        "arguments": arguments
    }, req_id)
    resp = read_response(process)
    if "error" in resp:
        raise Exception(f"Tool error: {resp['error']}")
    result_text = resp["result"]["content"][0]["text"]
    try:
        return json.loads(result_text)
    except:
        return result_text

def get_state(process):
    return call_tool(process, "get_state", {})

def type_text(process, text):
    return call_tool(process, "type_text", {"text": text})

def press_key(process, key):
    return call_tool(process, "press_key", {"key": key})

def wait_for_condition(process, condition_func, max_iterations=300, delay=0.05):
    for i in range(max_iterations):
        state = get_state(process)
        if condition_func(state):
            return True
        time.sleep(delay)
    return False

def main():
    game_path = "./output/warehouse_game.exe"
    
    print(f"Starting game: {game_path}", file=sys.stderr)
    process = subprocess.Popen(
        [game_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=sys.stderr,
        text=True,
        bufsize=1
    )

    try:
        req_id = 1
        
        send_request(process, "initialize", {}, req_id)
        resp = read_response(process)
        assert resp["result"]["serverInfo"]["name"] == "PrimePressureMCP"
        req_id += 1
        
        send_request(process, "tools/list", {}, req_id)
        resp = read_response(process)
        req_id += 1
        
        print("Waiting for game to initialize...", file=sys.stderr)
        time.sleep(2)
        
        print("Test: Waiting for order to be generated...", file=sys.stderr)
        order_generated = wait_for_condition(
            process,
            lambda state: len(state.get("orders", [])) > 0,
            max_iterations=1200,
            delay=0.05
        )
        
        if not order_generated:
            raise Exception("No order generated after waiting")
        
        print("Test: Order generated, checking current view...", file=sys.stderr)
        state = get_state(process)
        print(f"Current view: {state.get('view')}", file=sys.stderr)
        
        if state.get("view") != "computer":
            print("Warning: Not on computer view, but continuing...", file=sys.stderr)
        
        print("Test: Selecting first order (key 1)...", file=sys.stderr)
        state_before = get_state(process)
        print(f"State before key press - selected_order_id: {state_before.get('selected_order_id')}, orders: {len(state_before.get('orders', []))}", file=sys.stderr)
        
        # Press key multiple times to ensure it's processed
        for i in range(3):
            press_key(process, "1")
            req_id += 1
            time.sleep(0.1)  # Small delay between presses
        
        print("Test: Waiting for order to be selected...", file=sys.stderr)
        order_selected = wait_for_condition(
            process,
            lambda state: state.get("selected_order_id") is not None,
            max_iterations=200,  # Increase iterations significantly
            delay=0.05
        )
        
        if not order_selected:
            state_after = get_state(process)
            print(f"State after waiting - selected_order_id: {state_after.get('selected_order_id')}, orders: {json.dumps(state_after.get('orders'), indent=2)}", file=sys.stderr)
            raise Exception("Order was not selected after pressing key 1")
        
        print("Test: Order selected successfully!", file=sys.stderr)
        state = get_state(process)
        selected_order_id = state.get("selected_order_id")
        print(f"Selected order ID: {selected_order_id}", file=sys.stderr)
        print(f"Orders: {json.dumps(state.get('orders'), indent=2)}", file=sys.stderr)
        
        selected_order = None
        for order in state.get("orders", []):
            if order.get("id") == selected_order_id:
                selected_order = order
                break
        
        if not selected_order:
            raise Exception(f"Could not find selected order {selected_order_id} in state")
        
        if not selected_order.get("is_selected", False):
            raise Exception("Selected order does not have is_selected flag set")
        
        items = selected_order.get("items", [])
        if not items:
            raise Exception("Selected order has no items")
        
        print(f"Test: Selected order has {len(items)} items: {items}", file=sys.stderr)
        
        print("Test: Verifying order is still selected...", file=sys.stderr)
        state = get_state(process)
        if state.get("selected_order_id") != selected_order_id:
            raise Exception("Order was deselected unexpectedly")
        
        print("Test: Selecting order again (key 1) - should stay selected...", file=sys.stderr)
        press_key(process, "1")
        req_id += 1
        time.sleep(0.1)
        
        order_still_selected = wait_for_condition(
            process,
            lambda state: state.get("selected_order_id") == selected_order_id,
            max_iterations=60,
            delay=0.05
        )
        
        if not order_still_selected:
            raise Exception("Order was not still selected after pressing key 1 again")
        
        print("Test: Order still selected successfully!", file=sys.stderr)
        print("Test passed!", file=sys.stderr)
        
    except Exception as e:
        print(f"Test failed: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(1)
    finally:
        print("Terminating game...", file=sys.stderr)
        process.terminate()
        process.wait()

if __name__ == "__main__":
    main()

