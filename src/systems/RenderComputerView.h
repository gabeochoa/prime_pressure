#pragma once

#include <afterhours/ah.h>
#include <afterhours/src/plugins/files.h>

#include <algorithm>
#include <cmath>

#include "../components.h"
#include "../game.h"
#include "../order_state_machine.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"

struct RenderComputerViewSystem : ComputerViewRenderSystem {
   private:
    struct TimelineAtlas {
        raylib::Texture2D texture{};
        raylib::Rectangle conveyor{};
        raylib::Rectangle box{};
        raylib::Rectangle ready{};
        raylib::Rectangle ship{};
        bool loaded = false;
    };

    struct LayoutData {
        int screen_width;
        int screen_height;
        float screen_width_f;
        float screen_height_f;
        TriLayout layout;
        LayoutRect band;
        float box_x;
        float box_y;
        float box_width;
        float header_x;
        float header_y;
        float y_pixels;
        int body_font_size;
        int instruction_font_size;
        float pad_px;
        float content_x;
        float content_y;
        float content_w;
        int cols;
        float card_w;
        float card_h_base;
    };

    LayoutData setup_layout(
        [[maybe_unused]] const ActiveView &active_view) const {
        LayoutData data;
        data.screen_width = mainRT.texture.width;
        data.screen_height = mainRT.texture.height;
        data.screen_width_f = static_cast<float>(data.screen_width);
        data.screen_height_f = static_cast<float>(data.screen_height);

        data.layout = compute_tri_layout(data.screen_width, data.screen_height);
        data.band = data.layout.computer;

        data.box_x = data.band.x / data.screen_width_f;
        data.box_y = data.band.y / data.screen_height_f;
        data.box_width = data.band.w / data.screen_width_f;

        data.header_x = data.box_x + ui_constants::HEADER_PADDING_PCT;
        data.header_y = data.box_y + ui_constants::HEADER_PADDING_PCT;

        int header_font_size = ui_constants::pct_to_font_size(
            ui_constants::HEADER_FONT_SIZE_PCT, data.screen_height);
        float header_y_pixels =
            ui_constants::pct_to_pixels_y(data.header_y, data.screen_height);

        raylib::Vector2 header_text_size = raylib::MeasureTextEx(
            raylib::GetFontDefault(), "> [COMPUTER SCREEN]",
            static_cast<float>(header_font_size), 1.0f);
        float header_bottom_pixels = header_y_pixels + header_text_size.y;

        float spacing_pixels = ui_constants::pct_to_pixels_y(
            ui_constants::HEADER_TO_CONTENT_SPACING_PCT, data.screen_height);
        data.y_pixels = header_bottom_pixels + spacing_pixels;

        data.body_font_size = ui_constants::pct_to_font_size(
            ui_constants::BODY_FONT_SIZE_PCT, data.screen_height);
        data.instruction_font_size = ui_constants::pct_to_font_size(
            ui_constants::INSTRUCTION_FONT_SIZE_PCT, data.screen_height);

        data.pad_px = 16.0f;
        data.content_x = data.band.x + data.pad_px;
        data.content_y = data.y_pixels;
        data.content_w =
            (data.box_width * data.screen_width_f) - data.pad_px * 2.0f;
        float card_min_w = 280.0f;
        data.cols = std::max(
            1, std::min(3, static_cast<int>(data.content_w / card_min_w)));
        data.card_w = (data.content_w -
                       data.pad_px * (static_cast<float>(data.cols) + 1.0f)) /
                      static_cast<float>(data.cols);
        data.card_h_base = 140.0f;

        return data;
    }

    void draw_header_and_layout(const LayoutData &data,
                                const ActiveView &active_view) const {
        draw_view_box(data.band, active_view.current_view, ViewState::Computer);
        draw_view_header("COMPUTER SCREEN", data.header_x, data.header_y,
                         active_view.current_view, ViewState::Computer,
                         data.screen_width, data.screen_height);
    }

    void render_order_cards(const LayoutData &data,
                            const SelectedOrder &selected_order,
                            const ActiveOrder &active_order) const {
        // Collect orders with valid slots, sorted by slot index
        std::vector<std::pair<afterhours::EntityID, int>> slot_orders;
        for (const afterhours::Entity &entity :
             afterhours::EntityQuery()
                 .whereHasComponent<OrderSlot>()
                 .whereHasComponent<Order>()
                 .whereLambda([](const afterhours::Entity &e) {
                     const OrderSlot &slot = e.get<OrderSlot>();
                     return slot.index >= 0;
                 })
                 .gen()) {
            const OrderSlot &slot = entity.get<OrderSlot>();
            slot_orders.emplace_back(entity.id, slot.index);
        }

        if (slot_orders.empty()) {
            raylib::DrawTextEx(
                raylib::GetFontDefault(), "(No orders available)",
                raylib::Vector2{
                    data.content_x,
                    data.y_pixels +
                        static_cast<float>(data.instruction_font_size)},
                static_cast<float>(data.instruction_font_size), 1.0f,
                ui_colors::TERMINAL_GRAY);
            return;
        }

        // Sort by slot index
        std::sort(
            slot_orders.begin(), slot_orders.end(),
            [](const auto &a, const auto &b) { return a.second < b.second; });

        // Render non-selected orders first
        for (const auto &[order_id, slot_index] : slot_orders) {
            if (selected_order.order_id.order_id.has_value() &&
                selected_order.order_id.order_id.value() == order_id) {
                continue;
            }
            render_single_card(order_id, slot_index, data, selected_order,
                               active_order);
        }

        // Render selected order last (on top)
        for (const auto &[order_id, slot_index] : slot_orders) {
            if (!selected_order.order_id.order_id.has_value() ||
                selected_order.order_id.order_id.value() != order_id) {
                continue;
            }
            render_single_card(order_id, slot_index, data, selected_order,
                               active_order);
        }
    }

    void draw_card_items(const raylib::Rectangle &card_rect, float scaled_h,
                         [[maybe_unused]] float scaled_w,
                         const std::map<ItemType, int> &item_counts,
                         const std::map<ItemType, int> &ready_counts,
                         const LayoutData &data) const {
        float items_start_x = card_rect.x + 12.0f;
        float items_start_y = card_rect.y + 28.0f + 10.0f;  // title_h + 10
        float items_x = items_start_x;
        float items_y = items_start_y;
        float line_gap = static_cast<float>(data.body_font_size) + 6.0f;
        float max_items_y = card_rect.y + scaled_h - 44.0f;
        float column_gap = 24.0f;
        float column_width = 130.0f;
        float icon_size = 18.0f;
        int icon_font = std::max(8, data.body_font_size - 4);

        for (const auto &[item_type, count] : item_counts) {
            if (items_y + line_gap > max_items_y) {
                items_y = items_start_y;
                items_x += column_width + column_gap;
            }
            auto key_it = ITEM_KEY_MAP.find(item_type);
            std::string name = item_type_to_string(item_type);
            char icon_char =
                (key_it != ITEM_KEY_MAP.end())
                    ? static_cast<char>(std::toupper(
                          static_cast<unsigned char>(key_it->second)))
                    : (name.empty()
                           ? '?'
                           : static_cast<char>(std::toupper(
                                 static_cast<unsigned char>(name[0]))));
            auto ready_it = ready_counts.find(item_type);
            int ready_count =
                ready_it != ready_counts.end() ? ready_it->second : 0;
            bool is_item_ready = ready_count >= count;
            raylib::Color icon_fill = is_item_ready
                                          ? raylib::Color{91, 159, 59, 255}
                                          : raylib::Color{158, 180, 219, 255};

            raylib::Rectangle icon_rect{items_x, items_y, icon_size, icon_size};
            raylib::DrawRectangleRounded(icon_rect, 0.2f, 2, icon_fill);
            raylib::DrawRectangleRoundedLines(icon_rect, 0.2f, 2,
                                              raylib::Color{0, 0, 0, 80});
            std::string icon_text(1, icon_char);
            raylib::Vector2 icon_text_size = raylib::MeasureTextEx(
                raylib::GetFontDefault(), icon_text.c_str(),
                static_cast<float>(icon_font), 1.0f);
            float text_x = icon_rect.x + (icon_size - icon_text_size.x) * 0.5f;
            float text_y = icon_rect.y + (icon_size - icon_text_size.y) * 0.5f;
            raylib::DrawTextEx(raylib::GetFontDefault(), icon_text.c_str(),
                               raylib::Vector2{text_x, text_y},
                               static_cast<float>(icon_font), 1.0f,
                               raylib::WHITE);

            std::string item_line = name;
            raylib::DrawTextEx(raylib::GetFontDefault(), item_line.c_str(),
                               raylib::Vector2{items_x + icon_size + 8.0f,
                                               items_y + icon_size * 0.1f},
                               static_cast<float>(data.body_font_size), 1.0f,
                               raylib::Color{40, 50, 70, 255});

            items_y += line_gap;
        }
    }

    void draw_timeline(const raylib::Rectangle &card_rect, float scaled_h,
                       float scaled_w, OrderMacroState macro_state,
                       bool flash_toggle, const raylib::Color &sub_text,
                       const raylib::Color &accent_active,
                       [[maybe_unused]] const LayoutData &data,
                       OrderStateKind state_kind) const {
        float timeline_left = card_rect.x + 16.0f;
        float timeline_right = card_rect.x + scaled_w - 16.0f;
        float timeline_y = card_rect.y + scaled_h - 26.0f;
        int stages = 4;
        float segment_w = (timeline_right - timeline_left) /
                          static_cast<float>(std::max(1, stages - 1));

        auto get_timeline_atlas = []() -> TimelineAtlas & {
            static TimelineAtlas atlas;
            if (!atlas.loaded) {
                std::string path = afterhours::files::get_resource_path(
                                       "images/generated", "ui_atlas.png")
                                       .string();
                atlas.texture = raylib::LoadTexture(path.c_str());
                atlas.conveyor =
                    raylib::Rectangle{640.0f, 1472.0f, 48.0f, 48.0f};
                atlas.box = raylib::Rectangle{704.0f, 1472.0f, 48.0f, 48.0f};
                atlas.ready = raylib::Rectangle{768.0f, 1472.0f, 48.0f, 48.0f};
                atlas.ship = raylib::Rectangle{832.0f, 1472.0f, 48.0f, 48.0f};
                atlas.loaded = true;
            }
            return atlas;
        };

        TimelineAtlas &timeline_atlas = get_timeline_atlas();

        // Map macro state to stage index for rendering
        int current_stage_index = 0;  // Default to conveyor
        switch (macro_state) {
            case OrderMacroState::Incoming:
            case OrderMacroState::Opened:
            case OrderMacroState::RequestingItems:
            case OrderMacroState::ReceivingItems:
                current_stage_index = 0;  // Conveyor
                break;
            case OrderMacroState::ReadyToBox:
                current_stage_index = 1;  // Boxing
                break;
            case OrderMacroState::Boxing:
                current_stage_index = 1;  // Boxing
                break;
            case OrderMacroState::Shipped:
                current_stage_index = 3;  // Ship
                break;
            case OrderMacroState::Complete:
                current_stage_index = 3;  // Ship
                break;
            default:
                current_stage_index = 0;
                break;
        }

        // Determine flashing behavior based on state kind
        bool should_flash_timeline = (state_kind == OrderStateKind::Input);

        auto stage_color = [&](int stage_index, bool flashing) {
            // Color logic based on state kind and position
            if (stage_index == current_stage_index) {
                // Current stage
                if (state_kind == OrderStateKind::Terminal) {
                    // Complete - show green
                    return raylib::Color{91, 159, 59, 255};
                } else if (flashing && should_flash_timeline && flash_toggle) {
                    // Needs input - flash to indicate waiting
                    return raylib::Color{255, 200, 120, 255};
                } else {
                    // Processing or other - show active
                    return accent_active;
                }
            } else if (stage_index < current_stage_index) {
                // Previous stages - completed
                return raylib::Color{91, 159, 59, 255};  // Done - green
            } else {
                // Future/pending stages
                return sub_text;
            }
        };

        auto stage_for = [](int idx, bool is_done, bool is_active) {
            if (is_done) {
                return idx;  // Done stage index
            } else if (is_active) {
                return idx;  // Current stage index
            } else {
                return 99;  // Pending (some value > 3)
            }
        };

        auto stage_for_index = [&](int index) {
            bool is_done = (index < current_stage_index);
            bool is_active = (index == current_stage_index);
            return stage_for(index, is_done, is_active);
        };

        std::array<int, 4> states{stage_for_index(0), stage_for_index(1),
                                  stage_for_index(2), stage_for_index(3)};
        std::array<raylib::Rectangle, 4> sources{
            timeline_atlas.conveyor, timeline_atlas.box, timeline_atlas.ready,
            timeline_atlas.ship};
        float radius = 8.0f;

        draw_timeline_lines(timeline_left, timeline_right, timeline_y,
                            segment_w, stages, states, stage_color,
                            current_stage_index, should_flash_timeline, radius);
        draw_timeline_icons(timeline_left, timeline_right, timeline_y,
                            segment_w, stages, states, sources, timeline_atlas,
                            stage_color, current_stage_index,
                            should_flash_timeline);
    }

    void draw_timeline_lines(
        float timeline_left, [[maybe_unused]] float timeline_right,
        float timeline_y, float segment_w, int stages,
        const std::array<int, 4> &states,
        const std::function<raylib::Color(int, bool)> &stage_color,
        int current_stage_index, bool should_flash_timeline,
        float radius) const {
        for (int i = 0; i < stages - 1; ++i) {
            float x1 = timeline_left + segment_w * static_cast<float>(i);
            float x2 = timeline_left + segment_w * static_cast<float>(i + 1);
            // Determine line color based on stage states
            int p1_stage = states[i];
            int p2_stage = states[i + 1];

            bool p1_done = (p1_stage < current_stage_index);
            bool p1_active = (p1_stage == current_stage_index);
            bool p2_done = (p2_stage < current_stage_index);
            bool p2_active = (p2_stage == current_stage_index);

            int line_stage;
            if (p1_done && p2_done) {
                line_stage = 3;  // Done
            } else if (p1_active || p2_active) {
                line_stage = current_stage_index;  // Active
            } else {
                line_stage = 0;  // Pending
            }

            // Flash lines if the current state requires user input
            bool should_flash_line =
                should_flash_timeline && (p1_active || p2_active);
            raylib::DrawLineEx(raylib::Vector2{x1 + radius, timeline_y},
                               raylib::Vector2{x2 - radius, timeline_y}, 3.0f,
                               stage_color(line_stage, should_flash_line));
        }
    }

    void draw_timeline_icons(
        float timeline_left, [[maybe_unused]] float timeline_right,
        float timeline_y, float segment_w, int stages,
        const std::array<int, 4> &states,
        const std::array<raylib::Rectangle, 4> &sources,
        const TimelineAtlas &timeline_atlas,
        const std::function<raylib::Color(int, bool)> &stage_color,
        [[maybe_unused]] int current_stage_index,
        bool should_flash_timeline) const {
        for (int i = 0; i < stages; ++i) {
            float cx = timeline_left + segment_w * static_cast<float>(i);
            int stage_index = states[i];
            float icon_size_px = 28.0f;
            raylib::Rectangle dst{cx - icon_size_px * 0.5f,
                                  timeline_y - icon_size_px * 0.5f,
                                  icon_size_px, icon_size_px};
            // Flash current stage if it requires user input
            bool should_flash =
                (stage_index == current_stage_index) && should_flash_timeline;
            raylib::DrawTexturePro(timeline_atlas.texture, sources[i], dst,
                                   raylib::Vector2{0.0f, 0.0f}, 0.0f,
                                   stage_color(stage_index, should_flash));
        }
    }

    void draw_pack_banner(const raylib::Rectangle &card_rect, float scaled_w,
                          bool ready_to_pack, bool is_shipped,
                          const LayoutData &data) const {
        if (ready_to_pack && !is_shipped) {
            const char *pack_text = "READY TO PACK";
            int pack_font = data.instruction_font_size + 4;
            raylib::Color pack_color = raylib::Color{120, 90, 10, 255};
            raylib::Vector2 pack_size =
                raylib::MeasureTextEx(raylib::GetFontDefault(), pack_text,
                                      static_cast<float>(pack_font), 1.0f);
            raylib::Vector2 pack_pos{
                card_rect.x + (scaled_w - pack_size.x) * 0.5f,
                card_rect.y + card_rect.height - 26.0f - pack_size.y - 6.0f};
            raylib::DrawTextEx(raylib::GetFontDefault(), pack_text, pack_pos,
                               static_cast<float>(pack_font), 1.0f, pack_color);
        }
    }

    void draw_stamp_prompt(const raylib::Rectangle &card_rect, float scaled_h,
                           float scaled_w, bool is_shipped, int ready_stamp,
                           const LayoutData &data) const {
        if (!is_shipped) {
            return;
        }
        int stamp_font = data.instruction_font_size + 12;
        raylib::Color gray_hint = raylib::Color{140, 140, 140, 255};
        raylib::Color green_stamp = raylib::Color{50, 120, 50, 255};

        std::array<std::string, 3> stamp_words{"READY", "TO", "SHIP"};
        std::array<std::string, 3> stamp_keys{"R", "T", "S"};
        float padding = 8.0f;

        float total_width = 0.0f;
        for (size_t i = 0; i < stamp_words.size(); ++i) {
            raylib::Vector2 key_size = raylib::MeasureTextEx(
                raylib::GetFontDefault(), stamp_keys[i].c_str(),
                static_cast<float>(stamp_font), 1.0f);
            raylib::Vector2 word_size = raylib::MeasureTextEx(
                raylib::GetFontDefault(), stamp_words[i].c_str(),
                static_cast<float>(stamp_font), 1.0f);
            float key_w = key_size.x + padding * 2.0f;
            total_width += key_w + 10.0f + word_size.x;
            if (i < stamp_words.size() - 1) {
                total_width += 16.0f;
            }
        }
        float stamp_y = card_rect.y + scaled_h * 0.55f;
        float start_x = card_rect.x + (scaled_w - total_width) * 0.5f;

        float cursor_x = start_x;
        for (size_t i = 0; i < stamp_words.size(); ++i) {
            raylib::Color active_color =
                (ready_stamp >= static_cast<int>(i + 1)) ? green_stamp
                                                         : gray_hint;
            raylib::Vector2 key_size = raylib::MeasureTextEx(
                raylib::GetFontDefault(), stamp_keys[i].c_str(),
                static_cast<float>(stamp_font), 1.0f);
            float key_w = key_size.x + padding * 2.0f;
            draw_keycap(raylib::Vector2{cursor_x, stamp_y}, stamp_keys[i],
                        stamp_font, raylib::Color{24, 24, 24, 230},
                        active_color, raylib::GetFontDefault(), padding);
            cursor_x += key_w + 10.0f;

            raylib::Vector2 word_size = raylib::MeasureTextEx(
                raylib::GetFontDefault(), stamp_words[i].c_str(),
                static_cast<float>(stamp_font), 1.0f);
            raylib::DrawTextEx(raylib::GetFontDefault(), stamp_words[i].c_str(),
                               raylib::Vector2{cursor_x, stamp_y},
                               static_cast<float>(stamp_font), 1.0f,
                               active_color);
            cursor_x += word_size.x + 16.0f;
        }
    }

    void render_single_card(afterhours::EntityID order_id, int order_index,
                            const LayoutData &data,
                            const SelectedOrder &selected_order,
                            const ActiveOrder &active_order) const {
        if (order_id == -1) {
            return;
        }
        bool is_selected = selected_order.order_id.order_id.has_value() &&
                           selected_order.order_id.order_id.value() == order_id;
        bool is_active = active_order.order_id.order_id.has_value() &&
                         active_order.order_id.order_id.value() == order_id;

        const OrderWorkflow *workflow_ptr = nullptr;
        for (const afterhours::Entity &entity :
             afterhours::EntityQuery()
                 .whereID(order_id)
                 .whereHasComponent<Order>()
                 .whereHasComponent<OrderWorkflow>()
                 .gen()) {
            const Order &order = entity.get<Order>();
            workflow_ptr = &entity.get<OrderWorkflow>();
            OrderStateKind state_kind = kind_of(workflow_ptr->state);
            int row = order_index / data.cols;
            int col = order_index % data.cols;
            float card_h = std::max(is_selected ? data.card_h_base + 24.0f
                                                : data.card_h_base - 24.0f,
                                    100.0f);
            float card_x =
                data.content_x + data.pad_px +
                static_cast<float>(col) * (data.card_w + data.pad_px);
            float card_y = data.content_y +
                           static_cast<float>(row) * (card_h + data.pad_px);

            std::map<ItemType, int> item_counts = count_items(order.items);
            std::map<ItemType, int> selected_counts =
                count_items(order.selected_items);
            std::map<ItemType, int> ready_counts =
                count_items(order.ready_items);
            int ready_total = 0;
            for (const auto &[item_type, count] : ready_counts) {
                ready_total += count;
            }

            int total_items = static_cast<int>(order.items.size());
            // Derive values from workflow state instead of timeline
            bool is_shipped = macro_state_of(workflow_ptr->state) ==
                                  OrderMacroState::Shipped ||
                              macro_state_of(workflow_ptr->state) ==
                                  OrderMacroState::Complete;
            int ready_stamp = 0;  // Default
            if (workflow_ptr->state == OrderState::Shipped_Stamp0)
                ready_stamp = 0;
            else if (workflow_ptr->state == OrderState::Shipped_Stamp1)
                ready_stamp = 1;
            else if (workflow_ptr->state == OrderState::Shipped_Stamp2)
                ready_stamp = 2;
            else if (workflow_ptr->state == OrderState::Shipped_Stamp3)
                ready_stamp = 3;
            bool ready_to_pack = macro_state_of(workflow_ptr->state) ==
                                 OrderMacroState::ReadyToBox;
            bool should_flash_conveyor =
                false;  // TODO: implement based on workflow state
            // Use macro state directly for rendering

            // Map macro state to stage index for rendering
            int current_stage_index;
            switch (macro_state_of(workflow_ptr->state)) {
                case OrderMacroState::Incoming:
                case OrderMacroState::Opened:
                case OrderMacroState::RequestingItems:
                case OrderMacroState::ReceivingItems:
                    current_stage_index = 0;  // Conveyor
                    break;
                case OrderMacroState::ReadyToBox:
                    current_stage_index = 1;  // Boxing
                    break;
                case OrderMacroState::Boxing:
                    current_stage_index = 1;  // Boxing
                    break;
                case OrderMacroState::Shipped:
                    current_stage_index = 3;  // Ship
                    break;
                case OrderMacroState::Complete:
                    current_stage_index = 3;  // Ship
                    break;
                default:
                    current_stage_index = 0;
                    break;
            }

            raylib::Color face = {236, 233, 216, 255};
            raylib::Color accent_light = {199, 216, 237, 255};
            raylib::Color accent_active = {49, 106, 197, 255};
            raylib::Color accent_inactive = {158, 180, 219, 255};
            raylib::Color accent_dark = {10, 36, 106, 255};
            raylib::Color edge_light = {255, 255, 255, 255};
            raylib::Color edge_dark = {98, 98, 98, 255};
            raylib::Color shadow = {0, 0, 0, 80};
            raylib::Color body_text = {40, 50, 70, 255};
            raylib::Color sub_text = {70, 70, 70, 255};
            raylib::Color progress_fill = accent_active;

            if (ready_total >= total_items && total_items > 0) {
                progress_fill = {91, 159, 59, 255};
            }
            if (is_shipped) {
                face = {222, 222, 222, 255};
                accent_light = {200, 200, 200, 255};
                accent_inactive = {170, 170, 170, 255};
                accent_dark = {90, 90, 90, 255};
                body_text = {100, 100, 100, 255};
                sub_text = {120, 120, 120, 255};
                progress_fill = {150, 150, 150, 255};
            }

            raylib::Color title_main =
                is_active ? accent_active
                          : (is_selected ? accent_active : accent_inactive);
            raylib::Color title_shadow =
                is_active ? accent_dark
                          : (is_selected ? accent_dark
                                         : raylib::Color{120, 130, 150, 255});
            float scale = is_active ? 1.08f : (is_selected ? 1.05f : 1.0f);
            float scaled_w = data.card_w * scale;
            float scaled_h = card_h * scale;
            float offset_x = (scaled_w - data.card_w) * 0.5f;
            float offset_y = (scaled_h - card_h) * 0.5f;

            float shadow_offset = 4.0f;
            raylib::Rectangle shadow_rect{card_x - offset_x + shadow_offset,
                                          card_y - offset_y + shadow_offset,
                                          scaled_w, scaled_h};
            raylib::DrawRectangleRounded(shadow_rect, 0.02f, 4, shadow);

            raylib::Rectangle card_rect{card_x - offset_x, card_y - offset_y,
                                        scaled_w, scaled_h};
            raylib::DrawRectangleRounded(card_rect, 0.02f, 4, face);

            float title_h = 28.0f;
            raylib::Rectangle title_rect{card_rect.x, card_rect.y, scaled_w,
                                         title_h};
            raylib::DrawRectangleGradientV(
                static_cast<int>(title_rect.x), static_cast<int>(title_rect.y),
                static_cast<int>(title_rect.width),
                static_cast<int>(title_rect.height), accent_light, title_main);
            raylib::DrawRectangle(
                static_cast<int>(title_rect.x),
                static_cast<int>(title_rect.y + title_rect.height - 1),
                static_cast<int>(title_rect.width), 1, title_shadow);

            raylib::DrawRectangleRoundedLines(card_rect, 0.02f, 4, edge_light);
            raylib::DrawRectangleRoundedLines(
                raylib::Rectangle{card_rect.x + 1.0f, card_rect.y + 1.0f,
                                  card_rect.width - 2.0f,
                                  card_rect.height - 2.0f},
                0.02f, 4, edge_dark);

            bool flash_toggle =
                (static_cast<int>(raylib::GetTime() * 2.0f) % 2) == 0;

            int title_font = data.body_font_size;
            std::string base_title = "Order " +
                                     std::to_string(order_index + 1) + " (" +
                                     std::to_string(total_items) + " item" +
                                     (total_items == 1 ? ")" : "s)");
            std::string action_text;
            if (should_flash_conveyor) {
                action_text = "conveyor input needed";
            } else if (ready_to_pack) {
                action_text = "boxing needed";
            }
            std::string title = base_title;
            if (!action_text.empty() && !flash_toggle) {
                title = "Order " + std::to_string(order_index + 1) + " - " +
                        action_text;
            }
            raylib::DrawTextEx(
                raylib::GetFontDefault(), title.c_str(),
                raylib::Vector2{card_rect.x + 10.0f, card_rect.y + 6.0f},
                static_cast<float>(title_font), 1.0f, raylib::WHITE);

            if (is_selected) {
                draw_card_items(card_rect, scaled_h, scaled_w, item_counts,
                                ready_counts, data);
            }

            draw_timeline(card_rect, scaled_h, scaled_w,
                          macro_state_of(workflow_ptr->state), flash_toggle,
                          sub_text, accent_active, data, state_kind);

            draw_pack_banner(card_rect, scaled_w, ready_to_pack, is_shipped,
                             data);
            draw_stamp_prompt(card_rect, scaled_h, scaled_w, is_shipped,
                              ready_stamp, data);
            break;
        }
    }  // End of entity query loop

   public:
    void once(float) const {
        const ActiveView &active_view = get_singleton_as<ActiveView>();
        const SelectedOrder &selected_order = get_singleton_as<SelectedOrder>();
        const ActiveOrder &active_order = get_singleton_as<ActiveOrder>();

        LayoutData data = setup_layout(active_view);
        draw_header_and_layout(data, active_view);
        render_order_cards(data, selected_order, active_order);
    }
};
