# Bug Report

## 1. Boxing workflow never enters `FoldBox`
- **Problem:** When a ready order is selected in the boxing view the state machine is initialized to `BoxingState::None`, and pressing `B` immediately jumps to `PutItems`. The intermediary `FoldBox` state and its tutorial text are never reached even though both the UI copy and the automated tests expect it.
- **Impact:** Tests such as `test_boxing_workflow` wait for the state to become `FoldBox`, so they will time out. Players also never see the "Press 'B' to fold the box" prompt, making the workflow inconsistent with the documented steps.
- **Evidence:**
```57:103:src/systems/ProcessBoxingInput.h
        if (order_is_ready) {
...          boxing_progress.order_id = order_id;
          boxing_progress.state = BoxingState::None;
...      if (boxing_progress.state == BoxingState::None) {
        boxing_progress.state = BoxingState::PutItems;
        return;
      }
```
```50:64:src/testing/tests/TestBoxingWorkflow.h
  TestApp::simulate_key(raylib::KEY_ONE);
...  co_await TestApp::wait_for_condition(
      []() { return TestApp::get_boxing_state() == BoxingState::FoldBox; }, 60);
  TestApp::simulate_key(raylib::KEY_B);
...  []() { return TestApp::get_boxing_state() == BoxingState::PutItems; },
```
- **Recommendation:** Set `boxing_progress.state` to `BoxingState::FoldBox` when an order is loaded for boxing, and only move to `PutItems` once the player presses `B` in that state.

## 2. No settings file can be written on first run
- **Problem:** `load_save_file` returns `false` without setting `data->loaded_from` when neither of the candidate files exist. Later `write_save_file()` tries to open whatever `loaded_from` contains (which is still the default empty path) and fails. `main.cpp` always calls `write_save_file()` regardless of whether loading succeeded, so the game can never create a fresh `settings.json`.
- **Impact:** New users cannot persist their screen/audio preferences; the save call prints an error every shutdown and nothing is written.
- **Evidence:**
```152:189:src/settings.cpp
    if (file_loc >= settings_places.size()) {
      ...
      log_warn("{}", buffer.str());
      return false;
    }
...  data->loaded_from = settings_places[file_loc];
```
```192:205:src/settings.cpp
void Settings::write_save_file() {
  std::ofstream ofs(data->loaded_from);
  if (!ofs.good()) {
    std::cerr << "write_json_config_file error: Couldn't open file "
                 "for writing: "
              << data->loaded_from << std::endl;
    return;
  }
```
```68:82:src/main.cpp
  Settings::get().load_save_file(screenWidth, screenHeight);
...
  game();

  Settings::get().write_save_file();
```
- **Recommendation:** When no settings file exists, set `loaded_from` to a default path under `files::get_save_path()` (and/or create the directory) so `write_save_file()` has somewhere to write, or skip the save if loading failed.

## 3. Boxing order picker never shows slot numbers or status colors
- **Problem:** `create_order_display_info()` calculates `order_text` (the slot number) and `text_color` (ready/selected coloring), but `draw_order_text()` ignores both and always renders only `status_text` in the default font color.
- **Impact:** Players see a list of identical strings like "- Ready" or "- Slot not in progress" without any indication of which number key should be pressed or which order is highlighted, making the picker unusable under load.
- **Evidence:**
```46:89:src/systems/RenderBoxingView.h
OrderDisplayInfo info;
  info.order_number = order_number;
  info.order_text = std::to_string(order_number) + "";
...  if (is_selected || is_ready) {
    info.text_color = ui_colors::TERMINAL_GREEN;
  } else {
    info.text_color = ui_colors::TERMINAL_GRAY;
  }
```
```92:103:src/systems/RenderBoxingView.h
static void draw_order_text(const OrderDisplayInfo &info, ... ) {
  ...
  raylib::Color status_color =
      ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
  raylib::DrawTextEx(uiFont, info.status_text.c_str(), ... , status_color);
}
```
- **Recommendation:** Render `info.order_text` (e.g., "1." ) before the status and use `info.text_color` when drawing so that selected/ready orders are distinguishable and players know which slot they are looking at.

## 4. Typing buffer panel is always highlighted as the active view
- **Problem:** `RenderTypingBuffer` hard-codes both parameters of `draw_view_box` to `ViewState::Computer`, never consulting the actual `ActiveView`. The border therefore stays in the "active" color even when the player is looking at the warehouse or boxing views.
- **Impact:** The view-highlighting system loses meaning—players always see the computer panel lit up, contradicting the navigation instructions at the bottom of each screen.
- **Evidence:**
```18:25:src/systems/RenderTypingBuffer.h
    float box_width = ui_constants::BOX_WIDTH_PCT;
    float box_height = ui_constants::TYPING_BUFFER_HEIGHT_PCT;

    draw_view_box(box_x, box_y, box_width, box_height, screen_width,
                  screen_height, ViewState::Computer, ViewState::Computer);
```
- **Recommendation:** Fetch the `ActiveView` singleton and pass `active_view.current_view` as the `current_view` argument so the typing buffer is only highlighted when the player is actually on the computer screen.

## 5. Conveyor UI merges items from different orders at the same X position
- **Problem:** `RenderWarehouseViewItems` indexes both the item list and the owning order by `ConveyorItem::x_position` (a float). All conveyor entities spawn at the same starting X (`CONVEYOR_START_X_PCT`) and unused items remain there, so as soon as two orders exist their stationary items share the same key. The last order processed overwrites `order_id_by_position[x_position]`, causing items from multiple orders to be treated as if they belonged to a single order (incorrect colors, wrong `is_selected` highlighting, misleading "moving" status checks).
- **Evidence:**
```35:56:src/systems/RenderWarehouseViewItems.h
    std::map<float, std::map<ItemType, int>> items_by_position;
    std::map<float, afterhours::EntityID> order_id_by_position;
...
      items_by_position[conveyor_item.x_position][conveyor_item.type]++;
      order_id_by_position[conveyor_item.x_position] = conveyor_item.order_id;
```
```44:48:src/ui_constants.h
constexpr float CONVEYOR_START_X_PCT = 0.005f;
constexpr float CONVEYOR_END_X_PCT = 1.0f;
```
- **Impact:** With two or more active orders, the belt UI regularly shows the wrong order ID (and therefore the wrong color) for items still sitting at the start of the belt, making it unclear which order a keypress will affect.
- **Recommendation:** Use a key that is unique per entity (e.g., `vertical_index` or the entity ID itself) when grouping items, and keep the owning order alongside each entry instead of overwriting by X coordinate.
