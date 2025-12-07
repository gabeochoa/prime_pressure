#pragma once

#include "../components.h"
#include "../input_wrapper.h"
#include "../ui_constants.h"
#include <afterhours/ah.h>

struct ProcessTypingInputSystem : afterhours::System<TypingBuffer> {
  bool should_run(float) override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view != ViewState::Cutscene;
  }
  bool should_run(float) const override {
    const afterhours::Entity &view_entity =
        afterhours::EntityHelper::get_singleton<ActiveView>();
    const ActiveView &active_view = view_entity.get<ActiveView>();
    return active_view.current_view != ViewState::Cutscene;
  }

  void for_each_with(afterhours::Entity &entity, TypingBuffer &buffer,
                     float dt) override {
    (void)entity;

    if (game_input::IsKeyPressed(raylib::KEY_ENTER)) {
      buffer.buffer.clear();
      buffer.status = TypingStatus::Idle;
      buffer.last_input_time = 0.0f;
      buffer.status_time = 0.0f;
      return;
    }

    int key = game_input::GetCharPressed();
    if (key > 0) {
      char c = static_cast<char>(key);
      bool is_alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');

      if (is_alpha) {
        buffer.buffer =
            static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        buffer.last_input_time = 0.0f;
        buffer.status_time = 0.0f;
        buffer.status = TypingStatus::Typing;
      }
    }

    buffer.last_input_time += dt;
    buffer.status_time += dt;

    if ((buffer.status == TypingStatus::Match ||
         buffer.status == TypingStatus::Error) &&
        buffer.status_time > ui_constants::STATUS_FLASH_TIME) {
      buffer.status_time = 0.0f;
      buffer.status =
          buffer.buffer.empty() ? TypingStatus::Idle : TypingStatus::Typing;
    }

    if (buffer.last_input_time > ui_constants::TYPING_BUFFER_TIMEOUT) {
      buffer.buffer.clear();
      buffer.status = TypingStatus::Idle;
      buffer.status_time = 0.0f;
      buffer.last_input_time = 0.0f;
    }
  }
};
