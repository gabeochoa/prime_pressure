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
    print(f"Sending: {json_str}", file=sys.stderr)
    process.stdin.write(json_str + "\n")
    process.stdin.flush()

def read_response(process):
    line = process.stdout.readline()
    if not line:
        return None
    print(f"Received: {line.strip()}", file=sys.stderr)
    return json.loads(line)

def main():
    game_path = "./output/warehouse_game.exe" # Adjust extension if needed
    
    print(f"Starting game: {game_path}", file=sys.stderr)
    process = subprocess.Popen(
        [game_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=sys.stderr, # Redirect stderr to console so we can see logs
        text=True,
        bufsize=1
    )

    try:
        # 1. Initialize
        send_request(process, "initialize", {}, 1)
        resp = read_response(process)
        assert resp["result"]["serverInfo"]["name"] == "PrimePressureMCP"
        print("Initialize passed!", file=sys.stderr)

        # 2. List Tools
        send_request(process, "tools/list", {}, 2)
        resp = read_response(process)
        tools = resp["result"]["tools"]
        tool_names = [t["name"] for t in tools]
        assert "get_state" in tool_names
        assert "type_text" in tool_names
        assert "press_key" in tool_names
        print("List Tools passed!", file=sys.stderr)

        # 3. Call get_state
        # Wait a bit for game to initialize
        time.sleep(2) 
        send_request(process, "tools/call", {"name": "get_state", "arguments": {}}, 3)
        resp = read_response(process)
        state = json.loads(resp["result"]["content"][0]["text"])
        print(f"Game State: {json.dumps(state, indent=2)}", file=sys.stderr)
        assert "view" in state
        assert "orders" in state
        print("get_state passed!", file=sys.stderr)

        # 4. Type text
        send_request(process, "tools/call", {"name": "type_text", "arguments": {"text": "hello"}}, 4)
        resp = read_response(process)
        assert "Typed: hello" in resp["result"]["content"][0]["text"]
        print("type_text passed!", file=sys.stderr)
        
        # Verify typing buffer
        send_request(process, "tools/call", {"name": "get_state", "arguments": {}}, 5)
        resp = read_response(process)
        state = json.loads(resp["result"]["content"][0]["text"])
        # Note: typing might not be reflected immediately if game loop is slow or buffer logic differs
        # But let's see what we get
        print(f"Buffer after typing: {state.get('typing_buffer')}", file=sys.stderr)

    except Exception as e:
        print(f"Test failed: {e}", file=sys.stderr)
    finally:
        print("Terminating game...", file=sys.stderr)
        process.terminate()
        process.wait()

if __name__ == "__main__":
    main()
