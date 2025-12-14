#pragma once

#include <afterhours/ah.h>

#include "../components.h"
#include "../game.h"
#include "../ui_constants.h"
#include "RenderSystemBase.h"
#include "RenderSystemHelpers.h"

struct OrderDisplayInfo {
    int order_number;
    std::string order_text;
    std::string status_text;
    raylib::Color text_color;
};

static std::pair<std::string, raylib::Color> get_boxing_status_text(
    const BoxingProgress &boxing_progress, const Order &order) {
    log_info("BoxingRender: Current boxing state: {}, items_placed: {}",
             static_cast<int>(boxing_progress.state),
             boxing_progress.items_placed);
    switch (boxing_progress.state) {
        case BoxingState::FoldBox:
            return {"Press 'B' to fold the box", ui_colors::TERMINAL_AMBER};
        case BoxingState::PutItems: {
            int total_items = static_cast<int>(order.ready_items.size());
            std::string text = "Press 'P' to put items in box (" +
                               std::to_string(boxing_progress.items_placed) +
                               "/" + std::to_string(total_items) + ")";
            return {text, ui_colors::TERMINAL_AMBER};
        }
        case BoxingState::Fold:
            return {"Press 'F' to fold", ui_colors::TERMINAL_AMBER};
        case BoxingState::Tape:
            return {"Press 'T' to tape", ui_colors::TERMINAL_AMBER};
        case BoxingState::Ship:
            return {
                "Press 'S' to ship",
                raylib::Color{91, 159, 59, 255}};  // Windows XP green accent
        case BoxingState::None:
            return {"Press 'B' to start boxing", ui_colors::TERMINAL_GRAY};
    }
    return {"", ui_colors::TERMINAL_GRAY};
}

static OrderDisplayInfo create_order_display_info(
    int order_number, const Order *order_ptr, afterhours::EntityID order_id,
    const BoxingProgress &boxing_progress) {
    OrderDisplayInfo info;
    info.order_number = order_number;
    info.order_text = std::to_string(order_number) + "";

    if (!order_ptr) {
        info.status_text = " - Slot not in progress";
        info.text_color =
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
        return info;
    }

    const Order &order = *order_ptr;

    if (order.is_shipped()) {
        info.status_text = " - Shipped";
        info.text_color =
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
        return info;
    }

    bool is_ready = all_items_ready(order);
    bool is_selected = boxing_progress.order_id.has_value() &&
                       boxing_progress.order_id.value() == order_id;

    if (is_selected || is_ready) {
        info.text_color =
            raylib::Color{49, 106, 197, 255};  // Windows XP active accent
    } else {
        info.text_color = ui_colors::TERMINAL_GRAY;
    }

    if (is_ready) {
        info.status_text = " - Ready";
    } else {
        int ready_count = static_cast<int>(order.ready_items.size());
        int total_count = static_cast<int>(order.items.size());
        info.status_text = " - Waiting for items (" +
                           std::to_string(ready_count) + "/" +
                           std::to_string(total_count) + ")";
    }

    return info;
}

static void draw_order_text(const OrderDisplayInfo &info, float box_x, float y,
                            int screen_width, int screen_height,
                            int font_size) {
    float order_x = ui_constants::pct_to_pixels_x(
        box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f, screen_width);
    float order_y = ui_constants::pct_to_pixels_y(y, screen_height);
    raylib::Color status_color =
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font);
    raylib::DrawTextEx(uiFont, info.status_text.c_str(),
                       raylib::Vector2{order_x, order_y},
                       static_cast<float>(font_size), 1.0f, status_color);
}

static bool are_all_orders_shipped_or_empty() {
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereHasTag(GameTag::IsInProgressOrder)
             .whereHasComponent<Order>()
             .gen()) {
        const Order &order = order_entity.get<Order>();
        if (!order.is_shipped()) {
            return false;
        }
    }
    return true;
}

static void render_order_selection_list(float box_x, float &y,
                                        const BoxingProgress &boxing_progress,
                                        int screen_width, int screen_height) {
    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    raylib::DrawTextEx(
        uiFont, "Select Order:",
        raylib::Vector2{
            ui_constants::pct_to_pixels_x(
                box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
            ui_constants::pct_to_pixels_y(y, screen_height)},
        static_cast<float>(body_font_size), 1.0f,
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
    y += ui_constants::ORDER_ITEM_SPACING_PCT;

    // Collect orders with slots, sorted by slot index
    std::vector<std::pair<afterhours::EntityID, int>> slot_orders;
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereHasTag(GameTag::IsInProgressOrder)
             .whereHasComponent<Order>()
             .whereHasComponent<OrderSlot>()
             .gen()) {
        const OrderSlot &slot = order_entity.get<OrderSlot>();
        slot_orders.emplace_back(order_entity.id, slot.index);
    }

    if (slot_orders.empty() || are_all_orders_shipped_or_empty()) {
        raylib::DrawTextEx(
            uiFont, "No orders available",
            raylib::Vector2{
                ui_constants::pct_to_pixels_x(
                    box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
                ui_constants::pct_to_pixels_y(y, screen_height)},
            static_cast<float>(instruction_font_size), 1.0f,
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
        return;
    }

    // Sort by slot index
    std::sort(slot_orders.begin(), slot_orders.end(),
              [](const auto &a, const auto &b) { return a.second < b.second; });

    for (const auto &[order_id, slot_index] : slot_orders) {
        for (const afterhours::Entity &order_entity :
             afterhours::EntityQuery()
                 .whereID(order_id)
                 .whereHasComponent<Order>()
                 .gen()) {
            const Order &order = order_entity.get<Order>();
            OrderDisplayInfo info = create_order_display_info(
                slot_index + 1, &order, order_id, boxing_progress);
            draw_order_text(info, box_x, y, screen_width, screen_height,
                            instruction_font_size);
            y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
            break;
        }
    }
}

static int count_unplaced_items(
    const std::vector<afterhours::EntityID> &boxing_items) {
    int count = 0;
    for (afterhours::EntityID item_id : boxing_items) {
        for (const BoxingItemStatus &boxing_item :
             afterhours::EntityQuery()
                 .whereID(item_id)
                 .whereHasComponent<BoxingItemStatus>()
                 .gen_as<BoxingItemStatus>()) {
            if (!boxing_item.is_placed) {
                count++;
            }
            break;
        }
    }
    return count;
}

static int count_ready_items_in_queue() {
    int total = 0;
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereHasTag(GameTag::IsInProgressOrder)
             .whereHasComponent<Order>()
             .gen()) {
        const Order &order = order_entity.get<Order>();
        if (!order.is_shipped() && !order.ready_items.empty()) {
            std::map<ItemType, int> item_counts =
                count_items(order.ready_items);
            total += static_cast<int>(item_counts.size());
        }
    }
    return total;
}

static void render_boxing_items_list(float left_x, float &y,
                                     const BoxingProgress &boxing_progress,
                                     int screen_width, int screen_height,
                                     int font_size) {
    std::map<ItemType, int> unplaced_counts;
    for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
        for (const BoxingItemStatus &boxing_item :
             afterhours::EntityQuery()
                 .whereID(item_id)
                 .whereHasComponent<BoxingItemStatus>()
                 .gen_as<BoxingItemStatus>()) {
            if (!boxing_item.is_placed) {
                unplaced_counts[boxing_item.type]++;
            }
            break;
        }
    }

    for (const auto &[item_type, count] : unplaced_counts) {
        std::string item_text = "[ ] " + item_type_to_string(item_type);
        if (count > 1) {
            item_text += " x" + std::to_string(count);
        }
        raylib::DrawTextEx(
            uiFont, item_text.c_str(),
            raylib::Vector2{ui_constants::pct_to_pixels_x(left_x, screen_width),
                            ui_constants::pct_to_pixels_y(y, screen_height)},
            static_cast<float>(font_size), 1.0f,
            ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
        y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
    }
}

static void render_queue_items_list(float left_x, float &y, int screen_width,
                                    int screen_height, int font_size) {
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereHasTag(GameTag::IsInProgressOrder)
             .whereHasComponent<Order>()
             .gen()) {
        const Order &order = order_entity.get<Order>();
        if (order.is_shipped() || order.ready_items.empty()) {
            continue;
        }

        std::map<ItemType, int> item_counts = count_items(order.ready_items);
        for (const auto &[item_type, count] : item_counts) {
            std::string item_text =
                item_type_to_string(item_type) + " x" + std::to_string(count);
            raylib::DrawTextEx(
                uiFont, item_text.c_str(),
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(left_x, screen_width),
                    ui_constants::pct_to_pixels_y(y, screen_height)},
                static_cast<float>(font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
            y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.7f;
        }
    }
}

static void render_items_list(float left_x, float start_y,
                              const BoxingProgress &boxing_progress,
                              int screen_width, int screen_height) {
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    float y = start_y;
    int items_to_display = 0;

    if (boxing_progress.order_id.has_value() &&
        !boxing_progress.boxing_items.empty()) {
        items_to_display = count_unplaced_items(boxing_progress.boxing_items);
    } else {
        items_to_display = count_ready_items_in_queue();
    }

    if (items_to_display == 0) {
        return;
    }

    raylib::DrawTextEx(
        uiFont, "Items:",
        raylib::Vector2{ui_constants::pct_to_pixels_x(left_x, screen_width),
                        ui_constants::pct_to_pixels_y(y, screen_height)},
        static_cast<float>(instruction_font_size), 1.0f,
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
    y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;

    if (boxing_progress.order_id.has_value() &&
        !boxing_progress.boxing_items.empty()) {
        render_boxing_items_list(left_x, y, boxing_progress, screen_width,
                                 screen_height, instruction_font_size);
    } else {
        render_queue_items_list(left_x, y, screen_width, screen_height,
                                instruction_font_size);
    }
}

static void draw_dotted_line_horizontal(float x, float y, float length,
                                        raylib::Color color, float dash_length,
                                        float gap_length) {
    float total_length = 0.0f;
    float current_x = x;
    while (total_length < length) {
        float segment_length = std::min(dash_length, length - total_length);
        raylib::DrawLine(static_cast<int>(current_x), static_cast<int>(y),
                         static_cast<int>(current_x + segment_length),
                         static_cast<int>(y), color);
        current_x += segment_length + gap_length;
        total_length += segment_length + gap_length;
    }
}

static void draw_dotted_line_vertical(float x, float y, float length,
                                      raylib::Color color, float dash_length,
                                      float gap_length) {
    float total_length = 0.0f;
    float current_y = y;
    while (total_length < length) {
        float segment_length = std::min(dash_length, length - total_length);
        raylib::DrawLine(static_cast<int>(x), static_cast<int>(current_y),
                         static_cast<int>(x),
                         static_cast<int>(current_y + segment_length), color);
        current_y += segment_length + gap_length;
        total_length += segment_length + gap_length;
    }
}

static void draw_dotted_rectangle(float x, float y, float width, float height,
                                  raylib::Color color, float dash_length = 5.0f,
                                  float gap_length = 5.0f) {
    draw_dotted_line_horizontal(x, y, width, color, dash_length, gap_length);
    draw_dotted_line_horizontal(x, y + height, width, color, dash_length,
                                gap_length);
    draw_dotted_line_vertical(x, y, height, color, dash_length, gap_length);
    draw_dotted_line_vertical(x + width, y, height, color, dash_length,
                              gap_length);
}

static void render_box(float center_x_pct, float center_y_pct, int screen_width,
                       int screen_height,
                       const BoxingProgress &boxing_progress) {
    float center_x = ui_constants::pct_to_pixels_x(center_x_pct, screen_width);
    float center_y = ui_constants::pct_to_pixels_y(center_y_pct, screen_height);
    float box_width = ui_constants::pct_to_pixels_x(0.15f, screen_width);
    float box_height = ui_constants::pct_to_pixels_y(0.2f, screen_height);
    float box_x = center_x - box_width / 2.0f;
    float box_y = center_y - box_height / 2.0f;
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    raylib::Color box_color = raylib::Color{210, 180, 140, 255};

    if (boxing_progress.state == BoxingState::None) {
        draw_dotted_rectangle(box_x, box_y, box_width, box_height, box_color);
        return;
    }

    bool is_closed = boxing_progress.state >= BoxingState::Tape;

    if (is_closed) {
        raylib::DrawRectangle(static_cast<int>(box_x), static_cast<int>(box_y),
                              static_cast<int>(box_width),
                              static_cast<int>(box_height), box_color);

        if (boxing_progress.state == BoxingState::Ship) {
            raylib::Color tape_color = raylib::Color{250, 248, 240, 255};
            float stripe_width = box_width;
            float stripe_height = box_height * 0.08f;
            float stripe_y = box_y + box_height / 2.0f - stripe_height / 2.0f;
            raylib::DrawRectangle(static_cast<int>(box_x),
                                  static_cast<int>(stripe_y),
                                  static_cast<int>(stripe_width),
                                  static_cast<int>(stripe_height), tape_color);
        }
    } else {
        raylib::DrawRectangleLines(static_cast<int>(box_x),
                                   static_cast<int>(box_y),
                                   static_cast<int>(box_width),
                                   static_cast<int>(box_height), box_color);
    }

    if (!is_closed && boxing_progress.state >= BoxingState::PutItems &&
        boxing_progress.order_id.has_value() &&
        !boxing_progress.boxing_items.empty()) {
        float item_y = box_y + 10.0f;
        for (afterhours::EntityID item_id : boxing_progress.boxing_items) {
            for (const BoxingItemStatus &boxing_item :
                 afterhours::EntityQuery()
                     .whereID(item_id)
                     .whereHasComponent<BoxingItemStatus>()
                     .gen_as<BoxingItemStatus>()) {
                if (boxing_item.is_placed) {
                    std::string item_text =
                        item_type_to_string(boxing_item.type);
                    raylib::DrawTextEx(
                        uiFont, item_text.c_str(),
                        raylib::Vector2{box_x + 10.0f, item_y},
                        static_cast<float>(instruction_font_size) * 0.8f, 1.0f,
                        ui_constants::get_theme_color(
                            afterhours::ui::Theme::Usage::Font));
                    item_y += instruction_font_size * 0.8f;
                }
                break;
            }
        }
    }

    raylib::Color key_bg = raylib::Color{24, 24, 24, 230};
    raylib::Color key_fg = raylib::Color{255, 180, 0, 255};
    raylib::Vector2 key_pos{box_x + box_width * 0.5f,
                            box_y + box_height * 0.1f};

    if (boxing_progress.state == BoxingState::PutItems) {
        key_pos.x -= 10.0f;
        key_pos.y = box_y + box_height * 0.35f;
        draw_keycap(key_pos, "P", instruction_font_size, key_bg, key_fg,
                    uiFont);
    } else if (boxing_progress.state == BoxingState::Fold) {
        key_pos.x -= 10.0f;
        draw_keycap(key_pos, "F", instruction_font_size, key_bg, key_fg,
                    uiFont);
    } else if (boxing_progress.state == BoxingState::Tape) {
        key_pos.x -= 10.0f;
        draw_keycap(key_pos, "T", instruction_font_size, key_bg, key_fg,
                    uiFont);
    } else if (boxing_progress.state == BoxingState::Ship) {
        key_pos.x -= 10.0f;
        draw_keycap(key_pos, "S", instruction_font_size,
                    raylib::Color{30, 90, 30, 240},
                    raylib::Color{255, 255, 255, 255}, uiFont);
    }
}

static void render_boxing_progress(float box_x, float &y,
                                   const BoxingProgress &boxing_progress,
                                   const Order &order, int screen_width,
                                   int screen_height) {
    int body_font_size = ui_constants::pct_to_font_size(
        ui_constants::BODY_FONT_SIZE_PCT, screen_height);
    int instruction_font_size = ui_constants::pct_to_font_size(
        ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);

    raylib::DrawTextEx(
        uiFont, "Boxing Order:",
        raylib::Vector2{
            ui_constants::pct_to_pixels_x(
                box_x + ui_constants::CONTENT_PADDING_PCT, screen_width),
            ui_constants::pct_to_pixels_y(y, screen_height)},
        static_cast<float>(body_font_size), 1.0f,
        ui_constants::get_theme_color(afterhours::ui::Theme::Usage::Font));
    y += ui_constants::ORDER_ITEM_SPACING_PCT;

    int total_items_ready = static_cast<int>(order.ready_items.size());
    std::string key_text = "B";
    std::string tail_text = "to start boxing";
    raylib::Color tail_color = ui_colors::TERMINAL_GRAY;

    switch (boxing_progress.state) {
        case BoxingState::FoldBox:
            key_text = "B";
            tail_text = "to fold the box";
            tail_color = ui_colors::TERMINAL_AMBER;
            break;
        case BoxingState::PutItems:
            key_text = "P";
            tail_text = "to put items in box (" +
                        std::to_string(boxing_progress.items_placed) + "/" +
                        std::to_string(total_items_ready) + ")";
            tail_color = ui_colors::TERMINAL_AMBER;
            break;
        case BoxingState::Fold:
            key_text = "F";
            tail_text = "to fold";
            tail_color = ui_colors::TERMINAL_AMBER;
            break;
        case BoxingState::Tape:
            key_text = "T";
            tail_text = "to tape";
            tail_color = ui_colors::TERMINAL_AMBER;
            break;
        case BoxingState::Ship:
            key_text = "S";
            tail_text = "to ship";
            tail_color = raylib::Color{50, 120, 50, 255};
            break;
        case BoxingState::None:
        default:
            key_text = "B";
            tail_text = "to start boxing";
            tail_color = ui_colors::TERMINAL_GRAY;
            break;
    }

    raylib::Vector2 key_pos{
        ui_constants::pct_to_pixels_x(
            box_x + ui_constants::CONTENT_PADDING_PCT * 1.5f, screen_width),
        ui_constants::pct_to_pixels_y(y, screen_height)};
    float padding = 6.0f;
    raylib::Vector2 key_size =
        raylib::MeasureTextEx(uiFont, key_text.c_str(),
                              static_cast<float>(instruction_font_size), 1.0f);
    float key_w = key_size.x + padding * 2.0f;

    draw_keycap(key_pos, key_text, instruction_font_size,
                raylib::Color{24, 24, 24, 230}, tail_color, uiFont, padding);

    raylib::DrawTextEx(
        uiFont, tail_text.c_str(),
        raylib::Vector2{key_pos.x + key_w + 10.0f,
                        ui_constants::pct_to_pixels_y(y, screen_height)},
        static_cast<float>(instruction_font_size), 1.0f, tail_color);
    y += ui_constants::ORDER_ITEM_SPACING_PCT * 0.8f;
}

struct RenderBoxingViewSystem : BoxingViewRenderSystem {
    void once(float) const override {
        const afterhours::Entity &view_entity =
            afterhours::EntityHelper::get_singleton<ActiveView>();
        const ActiveView &active_view = view_entity.get<ActiveView>();

        int screen_width = mainRT.texture.width;
        int screen_height = mainRT.texture.height;
        float screen_width_f = static_cast<float>(screen_width);
        float screen_height_f = static_cast<float>(screen_height);

        TriLayout layout = compute_tri_layout(screen_width, screen_height);

        float box_x = layout.boxing.x / screen_width_f;
        float box_y = layout.boxing.y / screen_height_f;
        float box_width = layout.boxing.w / screen_width_f;
        float box_height = layout.boxing.h / screen_height_f;

        raylib::DrawRectangleGradientV(static_cast<int>(layout.boxing.x),
                                       static_cast<int>(layout.boxing.y),
                                       static_cast<int>(layout.boxing.w),
                                       static_cast<int>(layout.boxing.h),
                                       raylib::Color{180, 168, 150, 235},
                                       raylib::Color{128, 116, 98, 235});

        draw_view_box(layout.boxing, active_view.current_view,
                      ViewState::Boxing);

        float header_x = box_x + ui_constants::HEADER_PADDING_PCT;
        float header_y = box_y + ui_constants::HEADER_PADDING_PCT;
        draw_view_header("BOXING SCREEN", header_x, header_y,
                         active_view.current_view, ViewState::Boxing,
                         screen_width, screen_height, uiFont);

        const afterhours::Entity &boxing_progress_entity =
            afterhours::EntityHelper::get_singleton<BoxingProgress>();
        const BoxingProgress &boxing_progress =
            boxing_progress_entity.get<BoxingProgress>();

        float content_start_y = header_y + ui_constants::HEADER_FONT_SIZE_PCT +
                                ui_constants::HEADER_TO_CONTENT_SPACING_PCT;

        float left_x = box_x + ui_constants::CONTENT_PADDING_PCT;
        render_items_list(left_x, content_start_y, boxing_progress,
                          screen_width, screen_height);

        float center_x = box_x + box_width / 2.0f;
        float center_y = box_y + box_height / 2.0f;
        render_box(center_x, center_y, screen_width, screen_height,
                   boxing_progress);

        float right_x = box_x + box_width * 0.6f;
        float y = content_start_y;

        // Check if shipping animation is active
        const afterhours::Entity &animation_entity =
            afterhours::EntityHelper::get_singleton<ShippingAnimation>();
        const ShippingAnimation &animation =
            animation_entity.get<ShippingAnimation>();
        bool is_shipping = animation.is_active;

        if (!boxing_progress.order_id.has_value()) {
            render_order_selection_list(right_x, y, boxing_progress,
                                        screen_width, screen_height);
        } else if (is_shipping) {
            log_info("BoxingRender: Shipping order {}",
                     boxing_progress.order_id.value());
            // During shipping, show minimal info
            int body_font_size = ui_constants::pct_to_font_size(
                ui_constants::BODY_FONT_SIZE_PCT, screen_height);
            raylib::DrawTextEx(
                uiFont, "Order Shipping:",
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(
                        right_x + ui_constants::CONTENT_PADDING_PCT,
                        screen_width),
                    ui_constants::pct_to_pixels_y(y, screen_height)},
                static_cast<float>(body_font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
            y += ui_constants::ORDER_ITEM_SPACING_PCT;
            // Show order info here if needed
        } else {
            for (const Order &order :
                 afterhours::EntityQuery()
                     .whereID(boxing_progress.order_id.value())
                     .whereHasComponent<Order>()
                     .gen_as<Order>()) {
                render_boxing_progress(right_x, y, boxing_progress, order,
                                       screen_width, screen_height);
                break;
            }
        }

        // If shipping, overlay a message
        if (is_shipping) {
            int instruction_font_size = ui_constants::pct_to_font_size(
                ui_constants::INSTRUCTION_FONT_SIZE_PCT, screen_height);
            raylib::DrawTextEx(
                uiFont, "Shipping order...",
                raylib::Vector2{
                    ui_constants::pct_to_pixels_x(0.5f, screen_width) - 100.0f,
                    ui_constants::pct_to_pixels_y(0.7f, screen_height)},
                static_cast<float>(instruction_font_size), 1.0f,
                ui_constants::get_theme_color(
                    afterhours::ui::Theme::Usage::Font));
        }

        // No instruction text needed - all views are always visible
    }
};
