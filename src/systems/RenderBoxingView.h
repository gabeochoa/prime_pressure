#pragma once

#include "../components.h"
#include "../game.h"
#include <afterhours/ah.h>

static std::pair<std::string, raylib::Color>
get_boxing_status_text(const BoxingProgress &boxing_progress,
                       const Order &order) {
  switch (boxing_progress.state) {
  case BoxingState::FoldBox:
    return {"Press 'B' to fold the box", raylib::YELLOW};
  case BoxingState::PutItems: {
    int total_items = static_cast<int>(order.selected_items.size());
    std::string text = "Press 'P' to put items in box (" +
                       std::to_string(boxing_progress.items_placed) + "/" +
                       std::to_string(total_items) + ")";
    return {text, raylib::YELLOW};
  }
  case BoxingState::Fold:
    return {"Press 'F' to fold", raylib::YELLOW};
  case BoxingState::Tape:
    return {"Press 'T' to tape", raylib::YELLOW};
  case BoxingState::Ship:
    return {"Press 'S' to ship", raylib::GREEN};
  case BoxingState::None:
    return {"", raylib::WHITE};
  }
  return {"", raylib::WHITE};
}

static void render_order_selection_list(float box_x, float &y,
                                        const OrderQueue &queue) {
  raylib::DrawText("Select order to box:", static_cast<int>(box_x + 20),
                   static_cast<int>(y), 20, raylib::WHITE);
  y += 40.0f;

  int order_number = 1;
  for (afterhours::EntityID order_id : queue.active_orders) {
    for (const afterhours::Entity &order_entity :
         afterhours::EntityQuery()
             .whereID(order_id)
             .whereHasComponent<Order>()
             .gen()) {
      const Order &order = order_entity.get<Order>();

      if (all_items_selected(order) && !order.is_shipped) {
        std::string order_text =
            std::to_string(order_number) + ". Ready to box";
        raylib::DrawText(order_text.c_str(), static_cast<int>(box_x + 30),
                         static_cast<int>(y), 18, raylib::GREEN);
        y += 25.0f;
      }
      break;
    }
    order_number++;
  }

  raylib::DrawText("[Press number key to select order]",
                   static_cast<int>(box_x + 20), static_cast<int>(y + 20), 16,
                   raylib::GRAY);
}

static void render_boxing_progress(float box_x, float &y,
                                   const BoxingProgress &boxing_progress,
                                   const Order &order) {
  raylib::DrawText("Boxing Order:", static_cast<int>(box_x + 20),
                   static_cast<int>(y), 20, raylib::WHITE);
  y += 40.0f;

  auto [status_text, status_color] =
      get_boxing_status_text(boxing_progress, order);
  raylib::DrawText(status_text.c_str(), static_cast<int>(box_x + 30),
                   static_cast<int>(y), 18, status_color);
  y += 30.0f;

  if (boxing_progress.state != BoxingState::PutItems) {
    return;
  }

  raylib::DrawText("Items to place:", static_cast<int>(box_x + 30),
                   static_cast<int>(y), 16, raylib::GRAY);
  y += 25.0f;

  std::map<ItemType, int> item_counts = count_items(order.selected_items);
  for (const auto &[item_type, count] : item_counts) {
    std::string item_text =
        "  " + item_type_to_string(item_type) + " x" + std::to_string(count);
    raylib::DrawText(item_text.c_str(), static_cast<int>(box_x + 40),
                     static_cast<int>(y), 14, raylib::WHITE);
    y += 20.0f;
  }
}

struct RenderBoxingView : afterhours::System<> {
  bool should_run(float) override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Boxing;
  }
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view == ViewState::Boxing;
  }

  void once(float) const override {
    int screen_width = mainRT.texture.width;
    int screen_height = mainRT.texture.height;

    float box_x = 20.0f;
    float box_y = 60.0f;
    float box_width = static_cast<float>(screen_width) - 40.0f;
    float box_height = static_cast<float>(screen_height) - 200.0f;

    raylib::DrawRectangleLines(static_cast<int>(box_x), static_cast<int>(box_y),
                               static_cast<int>(box_width),
                               static_cast<int>(box_height), raylib::GREEN);

    raylib::DrawText("> BOXING SCREEN", static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + 10), 24, raylib::GREEN);

    const afterhours::Entity &boxing_progress_entity =
        afterhours::EntityHelper::get_singleton<BoxingProgress>();
    const BoxingProgress &boxing_progress =
        boxing_progress_entity.get<BoxingProgress>();

    float y = box_y + 50.0f;

    if (!boxing_progress.order_id.has_value()) {
      const afterhours::Entity &queue_entity =
          afterhours::EntityHelper::get_singleton<OrderQueue>();
      const OrderQueue &queue = queue_entity.get<OrderQueue>();
      render_order_selection_list(box_x, y, queue);
    } else {
      for (const afterhours::Entity &order_entity :
           afterhours::EntityQuery()
               .whereID(boxing_progress.order_id.value())
               .whereHasComponent<Order>()
               .gen()) {
        const Order &order = order_entity.get<Order>();
        render_boxing_progress(box_x, y, boxing_progress, order);
        break;
      }
    }

    raylib::DrawText("[COMPUTER] [WAREHOUSE] [BOXING] (Press TAB to switch)",
                     static_cast<int>(box_x + 10),
                     static_cast<int>(box_y + box_height - 60), 16,
                     raylib::GRAY);
  }
};
