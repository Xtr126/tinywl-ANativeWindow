#include "aidl/android/hardware/input/common/Axis.h"
#include <aidl/tinywl/BnTinywlInput.h>
#include <android/keycodes.h>
#include <cstdint>
#include <stdint.h>
#include <wayland-server-protocol.h>

extern "C" {
  #include <wlr/interfaces/wlr_keyboard.h>
  #include <wlr/interfaces/wlr_pointer.h>
  #include <linux/input-event-codes.h>
  #include "tinywl.h"
}

const struct wlr_pointer_impl tinywl_pointer_impl = {
    .name = "tinywl-pointer",
};

const struct wlr_keyboard_impl tinywl_keyboard_impl = {
    .name = "tinywl-keyboard",
};

extern float PointerCoords_getAxisValue(const aidl::android::hardware::input::common::PointerCoords& coords, int32_t axis);

namespace tinywl {
  using namespace aidl::com::android::server::inputflinger;
  using namespace aidl::android::hardware::input::common;

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {
  private:
    struct tinywl_server* server;
    void sendPointerButtonEvent(const MotionEvent& in_event) {
         
      struct wlr_pointer_button_event wlr_event = {
          .pointer = &pointer,
          .time_msec = (uint32_t)in_event.eventTime,
          .button = AKEYCODE_BUTTON_1,
          .state = WL_POINTER_BUTTON_STATE_PRESSED,
      };

      switch (in_event.action) {
        case Action::BUTTON_PRESS:
            wlr_event.state = WL_POINTER_BUTTON_STATE_PRESSED;
            break;
        case Action::BUTTON_RELEASE:
            wlr_event.state = WL_POINTER_BUTTON_STATE_RELEASED;
            break;
        default:
            return;
      }

      switch (in_event.actionButton) {
        case Button::PRIMARY:
            wlr_event.button = BTN_LEFT;
            break;
        case Button::SECONDARY:
            wlr_event.button = BTN_RIGHT;
            break;
        default:
            return;
      }

      wl_signal_emit_mutable(&pointer.events.button, &wlr_event);
      wl_signal_emit_mutable(&pointer.events.frame, &pointer);
    }

    void sendPointerPosition(const MotionEvent& in_event) {
      float x = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::X));
      float y = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::Y));

      struct wlr_pointer_motion_absolute_event wlr_event = {
        .pointer = &pointer,
        .time_msec = static_cast<uint32_t>(in_event.eventTime),
        .x = x / width,
        .y = y / height,
      };

      wl_signal_emit_mutable(&pointer.events.motion_absolute, &wlr_event);
      wl_signal_emit_mutable(&pointer.events.frame, &pointer);
    }

    void sendScrollEvent(const MotionEvent& in_event) {
      float delta = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::Y));
      struct wlr_pointer_axis_event wlr_event = {
        .pointer = &pointer,
        .time_msec = static_cast<uint32_t>(in_event.eventTime),
        .source = WL_POINTER_AXIS_SOURCE_WHEEL,
        .orientation = WL_POINTER_AXIS_VERTICAL_SCROLL,
        .delta = delta,
        .delta_discrete = static_cast<int32_t>(delta * WLR_POINTER_AXIS_DISCRETE_STEP),
      };
      
      wl_signal_emit_mutable(&pointer.events.axis, &wlr_event);
      wl_signal_emit_mutable(&pointer.events.frame, &pointer);

    }

  public:
    ::ndk::ScopedAStatus onKeyEvent(const KeyEvent& in_event, bool* _aidl_return) override {
      std::cout << in_event.toString() << std::endl;
      
      if (in_event.source != Source::KEYBOARD) {
        return ::ndk::ScopedAStatus::ok();
      }

      struct wlr_keyboard_key_event wlr_event = {
          .time_msec = (uint32_t)in_event.eventTime,
          .keycode = (uint32_t)in_event.scanCode,
          .update_state = true,
      };

      switch (in_event.action) {
        case KeyEventAction::DOWN:
            wlr_event.state = WL_KEYBOARD_KEY_STATE_PRESSED;
            break;
        case KeyEventAction::UP:
            wlr_event.state = WL_KEYBOARD_KEY_STATE_RELEASED;
            break;
        default:
            // Skip other actions like AKEY_EVENT_ACTION_MULTIPLE
            return ::ndk::ScopedAStatus::ok();;
      }

      wlr_keyboard_notify_key(&keyboard, &wlr_event);
      *_aidl_return = true;
      return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onMotionEvent(const MotionEvent& in_event, bool* _aidl_return) override {
      std::cout << in_event.toString() << std::endl;
      
      if (in_event.source != Source::MOUSE) {
        return ::ndk::ScopedAStatus::ok();
      }

      switch (in_event.action) {
        case Action::BUTTON_PRESS:
        case Action::BUTTON_RELEASE:
          sendPointerButtonEvent(in_event);
          break;
        case Action::MOVE:
        case Action::HOVER_ENTER:
        case Action::HOVER_EXIT:
          sendPointerPosition(in_event);
          break;
        case Action::SCROLL:
          sendScrollEvent(in_event);
          break;
        default:
            // Skip other actions
            return ::ndk::ScopedAStatus::ok();;
      }

      return ::ndk::ScopedAStatus::ok();
    }

    void setTinywlServer(struct tinywl_server* server) {
      this->server = server;
      wlr_keyboard_init(&keyboard, &tinywl_keyboard_impl, "tinywl-keyboard");
      server->new_input.notify(&server->new_input, &keyboard.base);

      wlr_pointer_init(&pointer, &tinywl_pointer_impl, "tinywl-pointer");
      server->new_input.notify(&server->new_input, &pointer.base);
    }

    struct wlr_keyboard keyboard;

    struct wlr_pointer pointer;

    int32_t width;
    int32_t height;
  };  // class TinywlInputService

}  // namespace tinywl

std::shared_ptr<tinywl::TinywlInputService> TinywlInputService_make() { 
  return ndk::SharedRefBase::make<tinywl::TinywlInputService>();
}